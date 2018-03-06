#ifndef REPO_NDN_GET_META_HPP
#define REPO_NDN_GET_META_HPP
#include <ndn-cxx/face.hpp>
namespace repo {
class VideoGetMeta : ndn::noncopyable {
public:
    VideoGetMeta(const std::string& name)
            :m_dataName(name), m_interestLifetime(ndn::time::milliseconds(5000))
    {
    }
    int fetchData() {
        ndn::Interest interest(m_dataName);
        interest.setInterestLifetime(m_interestLifetime);
        interest.setMustBeFresh(true);
        m_face.expressInterest(interest,
                                bind(&VideoGetMeta::onData, this, _2),
                                bind(&VideoGetMeta::onTimeout,this,_1));
        m_face.processEvents(this->m_interestLifetime);
        return frame_num;
    }
private:
    void onData(const ndn::Data& data) {
        frame_num = *((uint32_t *)(data.getContent().value()));
        std::cout << "consumer: " << frame_num << std::endl;
    }
    void onTimeout(const ndn::Interest& interest) {
        fetchData();
    }
private:
    ndn::Face m_face;
    ndn::Name m_dataName;
    ndn::time::milliseconds m_interestLifetime;
    uint32_t frame_num;
};
}
#endif
