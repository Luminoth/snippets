#if !defined __VERSION_H__
#define __VERSION_H__

namespace energonsoftware {

class Version
{
public:
    explicit Version(const std::string& version);
    virtual ~Version() throw();

public:
    unsigned int major() const { return _major; }
    unsigned int minor() const { return _minor; }
    unsigned int maintenance() const { return _maintenance; }

    std::string str() const;

public:
    bool operator<(const Version& rhs) const;
    bool operator<=(const Version& rhs) const;
    bool operator>(const Version& rhs) const;
    bool operator>=(const Version& rhs) const;
    bool operator==(const Version& rhs) const;
    bool operator!=(const Version& rhs) const;

private:
    unsigned int _major, _minor, _maintenance;

private:
    Version();
};

}

#endif
