#ifndef REPO_NG_TOOLS_NDNGETFILE_HPP
#define REPO_NG_TOOLS_NDNGETFILE_HPP
#include <ndn-cxx/face.hpp>
#include "pipe.h"
namespace repo {
class Consumer : boost::noncopyable
{
public:
  Consumer(const std::string& dataName, std::ostream& os, pipe_producer_t* _pro,
           bool verbose, bool versioned, bool single,
           int interestLifetime, int timeout,
           bool mustBeFresh = false)
    : m_dataName(dataName)
    , m_os(os)
    , m_pipe_pro(_pro)
    , m_verbose(verbose)
    , m_hasVersion(versioned)
    , m_isSingle(single)
    , m_isFinished(false)
    , m_isFirst(true)
    , m_interestLifetime(interestLifetime)
    , m_timeout(timeout)
    , m_nextSegment(0)
    , m_totalSize(0)
    , m_retryCount(0)
    , m_mustBeFresh(mustBeFresh)
  {
  }
  void
  run();
private:
  void
  fetchData(const ndn::Name& name);
  void
  onVersionedData(const ndn::Interest& interest, ndn::Data& data);
  void
  onUnversionedData(const ndn::Interest& interest, ndn::Data& data);
  void
  onTimeout(const ndn::Interest& interest);
  
  void
  readData(const ndn::Data& data);
  void
  fetchNextData(const ndn::Name& name, const ndn::Data& data);
private:
  ndn::Face m_face;
  ndn::Name m_dataName;
  std::ostream& m_os;
  pipe_producer_t* m_pipe_pro;
  bool m_verbose;
  bool m_hasVersion;
  bool m_isSingle;
  bool m_isFinished;
  bool m_isFirst;
  ndn::time::milliseconds m_interestLifetime;
  ndn::time::milliseconds m_timeout;
  uint64_t m_nextSegment;
  int m_totalSize;
  int m_retryCount;
  bool m_mustBeFresh;
};
} // namespace repo
#endif // REPO_NG_TOOLS_NDNGETFILE_HPP
