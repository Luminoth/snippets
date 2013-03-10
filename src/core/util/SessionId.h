#if !defined __SESSIONID_H__
#define __SESSIONID_H__

namespace energonsoftware {

class SessionId final
{
public:
    // expiry == -1 means no expiration
    explicit SessionId(const std::string& password, int expiry=-1);
    SessionId(const std::string& password, const std::string& sessionid, int expiry=-1);
    virtual ~SessionId() noexcept;

public:
    const std::string& password() const { return _password; }
    int expiry() const { return _expiry; }
    const std::string& sessionid() const { return _sessionid; }
    long creation_time() const { return _creation_time; }
    bool expired() const;

private:
    std::string _password;
    int _expiry;
    std::string _sessionid;
    long _creation_time;

private:
    SessionId();
};

}

#endif
