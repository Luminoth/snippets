#if !defined __NONCE_H__
#define __NONCE_H__

namespace energonsoftware {

class Nonce final
{
public:
    Nonce(const std::string& realm, int expiry);
    Nonce(const std::string& realm, const std::string& nonce, int expiry);
    virtual ~Nonce() throw();

public:
    const std::string& realm() const { return _realm; }
    int expiry() const { return _expiry; }
    const std::string& nonce() const { return _nonce; }
    const std::string& md5() const { return _nonce_md5; }
    long creation_time() const { return _creation_time; }
    bool expired() const;

private:
    void calc_md5();

private:
    std::string _realm;
    int _expiry;
    std::string _nonce;
    std::string _nonce_md5;
    long _creation_time;

private:
    Nonce();
};

}

#endif
