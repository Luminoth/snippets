#if !defined __BUFFEREDMESSAGE_H__
#define __BUFFEREDMESSAGE_H__

namespace energonsoftware {

enum /*class*/ BufferedMessageType
{
    // these are handleable
    Xml,
    Binary,

    // these are not handlable
    Unhandlable,
    Udp,
    String,
    Delayed
};

class BufferedMessage
{
public:
    explicit BufferedMessage(bool encode);

    // NOTE: this does not copy the data, so reset() must be called before using it
    BufferedMessage(const BufferedMessage& message);

    virtual ~BufferedMessage() noexcept;

public:
    // returns true if the data should be encoded before sending
    virtual bool encode() const final { return _encode; }

    // returns a pointer to the actual start of the data
    // reset() must have been called once before this is valid
    virtual const unsigned char* start() const final { return _data.get(); }

    // returns a pointer to the current start of the data
    // reset() must have been called once before this is valid
    virtual const unsigned char* current() const final { return _data_ptr; }

    // returns the length of the full data
    // reset() must have been called once before this is valid
    virtual size_t full_len() const final { return _data_len; }

    // returns the current length of the data
    // reset() must have been called once before this is valid
    virtual size_t len() const final { return _data_len - (_data_ptr - _data.get()); }

    // resets the data pointer to the start
    virtual void reset() final;

    // advances the data pointer by some amount
    virtual void advance(size_t len) final;

    // returns true if we've hit the end of the data
    virtual bool finished() const final;

    // used for sequencing messages
    virtual long seqid() const final { return _seqid; }
    virtual void seqid(long id) final { _seqid = id; }

    // NOTE: this does not copy the data, so reset() must be called before using it
    virtual BufferedMessage& operator=(const BufferedMessage& rhs) final;

public:
    // override these
    virtual BufferedMessageType msg_type() const = 0;
    virtual bool has_seqid() const { return false; }

protected:
    // this is copied by the caller, so it doesn't have to be heap storage
    virtual const unsigned char* data() /*const*/ = 0;
    virtual size_t data_len() const = 0;

private:
    bool _encode;
    std::shared_ptr<unsigned char> _data;
    const unsigned char* _data_ptr;
    size_t _data_len;
    long _seqid;

private:
    BufferedMessage();
};

}

#endif
