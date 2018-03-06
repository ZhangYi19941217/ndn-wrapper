#include "ndngetfile.hpp"
#include "ndninterface.h"
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <string>
namespace repo {
    using ndn::Name;
    using ndn::Interest;
    using ndn::Data;
    using ndn::Block;
    using std::bind;
    using std::placeholders::_1;
    using std::placeholders::_2;
    static const int MAX_RETRY = 3;
    void
    Consumer::fetchData(const Name &name) {
      Interest interest(name);
      interest.setInterestLifetime(m_interestLifetime);
      //std::cout<<"interest name = "<<interest.getName()<<std::endl;
      if (m_hasVersion) {
        interest.setMustBeFresh(m_mustBeFresh);
      } else {
        interest.setMustBeFresh(true);
        interest.setChildSelector(1);
      }
      m_face.expressInterest(interest,
                             m_hasVersion ?
                             bind(&Consumer::onVersionedData, this, _1, _2)
                                          :
                             bind(&Consumer::onUnversionedData, this, _1, _2),
                             bind(&Consumer::onTimeout, this, _1));
    }
    void
    Consumer::run() {
      // Send the first Interest
      Name name(m_dataName);
      m_nextSegment++;
      fetchData(name);
      // processEvents will block until the requested data received or timeout occurs
      m_face.processEvents(m_timeout);
    }
    void
    Consumer::onVersionedData(const Interest &interest, Data &data) {
      const Name &name = data.getName();
      // the received data name may have segment number or not
      if (name.size() == m_dataName.size()) {
        if (!m_isSingle) {
          Name fetchName = name;
          fetchName.appendSegment(0);
          fetchData(fetchName);
        }
      } else if (name.size() == m_dataName.size() + 1) {
        if (!m_isSingle) {
          if (m_isFirst) {
            uint64_t segment = name[-1].toSegment();
            if (segment != 0) {
              fetchData(Name(m_dataName).appendSegment(0));
              m_isFirst = false;
              return;
            }
            m_isFirst = false;
          }
          fetchNextData(name, data);
        } else {
          std::cerr << "ERROR: Data is not stored in a single packet" << std::endl;
          return;
        }
      } else {
        std::cerr << "ERROR: Name size does not match" << std::endl;
        return;
      }
      readData(data);
    }
    void
    Consumer::onUnversionedData(const Interest &interest, Data &data) {
      const Name &name = data.getName();
      //std::cout<<"recevied data name = "<<name<<std::endl;
      if (name.size() == m_dataName.size() + 1) {
        if (!m_isSingle) {
          Name fetchName = name;
          fetchName.append(name[-1]).appendSegment(0);
          fetchData(fetchName);
        }
      } else if (name.size() == m_dataName.size() + 2) {
        if (!m_isSingle) {
          if (m_isFirst) {
            uint64_t segment = name[-1].toSegment();
            if (segment != 0) {
              fetchData(Name(m_dataName).append(name[-2]).appendSegment(0));
              m_isFirst = false;
              return;
            }
            m_isFirst = false;
          }
          fetchNextData(name, data);
        } else {
          std::cerr << "ERROR: Data is not stored in a single packet" << std::endl;
          return;
        }
      } else {
        std::cerr << "ERROR: Name size does not match" << std::endl;
        return;
      }
      readData(data);
    }
    void
    Consumer::readData(const Data &data) {
      const Block &content = data.getContent();
      //m_os.write(reinterpret_cast<const char *>(content.value()), content.value_size());
      pipe_push(m_pipe_pro, reinterpret_cast<const char *>(content.value()), content.value_size());
      m_totalSize += content.value_size();
      if (m_verbose) {
        std::cerr << "LOG: received data = " << data.getName() << std::endl;
      }
      if (m_isFinished || m_isSingle) {
        std::cerr << "INFO: End of file is reached." << std::endl;
        std::cerr << "INFO: Total # of segments received: " << m_nextSegment << std::endl;
        std::cerr << "INFO: Total # bytes of content received: " << m_totalSize << std::endl;
      }
    }
    void
    Consumer::fetchNextData(const Name &name, const Data &data) {
      uint64_t segment = name[-1].toSegment();
      BOOST_ASSERT(segment == (m_nextSegment - 1));
      const ndn::name::Component &finalBlockId = data.getMetaInfo().getFinalBlockId();
      if (finalBlockId == name[-1]) {
        m_isFinished = true;
      } else {
        // Reset retry counter
        m_retryCount = 0;
        if (m_hasVersion)
          fetchData(Name(m_dataName).appendSegment(m_nextSegment++));
        else
          fetchData(Name(m_dataName).append(name[-2]).appendSegment(m_nextSegment++));
      }
    }
    void
    Consumer::onTimeout(const Interest &interest) {
      if (m_retryCount++ < MAX_RETRY) {
        // Retransmit the interest
        fetchData(interest.getName());
        if (m_verbose) {
          std::cerr << "TIMEOUT: retransmit interest for " << interest.getName() << std::endl;
        }
      } else {
        std::cerr << "TIMEOUT: last interest sent for segment #" << (m_nextSegment - 1) << std::endl;
        std::cerr << "TIMEOUT: abort fetching after " << MAX_RETRY
                  << " times of retry" << std::endl;
      }
    }
}
extern "C" {
void getbyname(const char* name, pipe_producer_t* pro_) {
    char* outputFile;
    std::cout << "name: " << name << "file: " << outputFile << std::endl;
    bool verbose = false, versioned = false, single = false;
    int interestLifetime = 4000;  // in milliseconds
    int timeout = 0;  // in milliseconds
    std::streambuf* buf;
    std::ofstream of;
    outputFile = 0;
    if (outputFile != 0)
    {
        of.open(outputFile, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!of || !of.is_open()) {
            std::cerr << "ERROR: cannot open " << outputFile << std::endl;
        }
        buf = of.rdbuf();
    }
    else
    {
        buf = std::cout.rdbuf();
    }
    std::ostream os(buf);
    std::string interest_name(name);
    repo::Consumer consumer(interest_name, os, pro_, verbose, versioned, single,
                            interestLifetime, timeout);
    try
    {
        consumer.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }
}
}
