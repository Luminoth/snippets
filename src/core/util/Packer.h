#if !defined __PACKER_H__
#define __PACKER_H__

namespace energonsoftware {

class Serializable;

class PackerError : public std::exception
{
public:
    explicit PackerError(const std::string& what) noexcept : _what(what) {}
    virtual ~PackerError() noexcept {}
    virtual const char* what() const noexcept { return _what.c_str(); }

private:
    std::string _what;
};

enum class PackerType
{
    Simple,
    Binary,
    //Protobuf,
    XML,
};

class Packer
{
public:
    static std::string type_to_str(PackerType type);

    static std::shared_ptr<Packer> new_packer(PackerType type, const boost::any& data=boost::any(std::string("")));

private:
    static Logger& logger;

public:
    Packer() {}
    virtual ~Packer() noexcept {}

public:
    template<typename T>
    /*virtual*/ Packer& pack(const std::list<T>& v, const std::string& name) throw(PackerError) /*final*/
    {
        pack(static_cast<uint32_t>(v.size()), name + "_count");
        for(const T& item : v) {
            pack(item, name);
        }
        return *this;
    }

    template<typename T>
    /*virtual*/ Packer& pack(const std::vector<T>& v, const std::string& name) throw(PackerError) /*final*/
    {
        pack(static_cast<uint32_t>(v.size()), name + "_count");
        for(const T& item : v) {
            pack(item, name);
        }
        return *this;
    }

    template<typename T>
    /*virtual*/ Packer& pack(const std::deque<T>& v, const std::string& name) throw(PackerError) /*final*/
    {
        pack(static_cast<uint32_t>(v.size()), name + "_count");
        for(const T& item : v) {
            pack(item, name);
        }
        return *this;
    }

#if 0
    template<typename K, typename V>
    /*virtual*/ Packer& pack(const std::map<K, V>& v, const std::string& name) throw(PackerError) /*final*/
    {
        pack(static_cast<uint32_t>(v.size()(, name + "_count");

        for(const auto& i : v) {
            pack(i.first, name + "_key");
            pack(i.second, name + "_value");
        }
        return *this;
    }

    template<typename K, typename V>
    /*virtual*/ Packer& pack(const std::unordered_map<K, V>& v, const std::string& name) throw(PackerError) /*final*/
    {
        pack(static_cast<uint32_t>(v.size()), name + "_count");

        for(const auto& i : v) {
            pack(i.first, name + "_key");
            pack(i.second, name + "_value");
        }
        return *this;
    }
#endif

    template<typename T>
    /*virtual*/ Packer& pack(const std::shared_ptr<T>& t, const std::string& name) throw(PackerError) /*final*/
    {
        pack(*t, name);
        return *this;
    }

    virtual Packer& pack(const Serializable& v, const std::string& name) throw(PackerError) final;

public:
    virtual Packer& reset() = 0;
    virtual Packer& pack(const std::string& v, const std::string& name) throw(PackerError) = 0;
    virtual Packer& pack(const char* const v, const std::string& name) throw(PackerError) = 0;
    virtual Packer& pack(int8_t v, const std::string& name) throw(PackerError) = 0;
    virtual Packer& pack(uint8_t v, const std::string& name) throw(PackerError) = 0;
    /*virtual Packer& pack(int16_t v, const std::string& name) throw(PackerError) = 0;
    virtual Packer& pack(uint16_t v, const std::string& name) throw(PackerError) = 0;*/
    virtual Packer& pack(int32_t v, const std::string& name) throw(PackerError) = 0;
    virtual Packer& pack(uint32_t v, const std::string& name) throw(PackerError) = 0;
    virtual Packer& pack(int64_t v, const std::string& name) throw(PackerError) = 0;
    virtual Packer& pack(uint64_t v, const std::string& name) throw(PackerError) = 0;
    virtual Packer& pack(float v, const std::string& name) throw(PackerError) = 0;
    virtual Packer& pack(double v, const std::string& name) throw(PackerError) = 0;
    virtual Packer& pack(bool v, const std::string& name) throw(PackerError) = 0;
    virtual const std::string buffer() const = 0;

private:
    DISALLOW_COPY_AND_ASSIGN(Packer);
};

class Unpacker
{
public:
    static std::shared_ptr<Unpacker> new_unpacker(const std::string& obj, PackerType type, const boost::any& data=boost::any(std::string("")));
    static std::shared_ptr<Unpacker> new_unpacker(const unsigned char* obj, size_t len, PackerType type, const boost::any& data=boost::any(std::string("")));

private:
    static Logger& logger;

public:
    explicit Unpacker(const std::string& obj) : _obj(obj) {}
    explicit Unpacker(const std::vector<unsigned char>& obj);
    Unpacker(const unsigned char* obj, size_t len);
    virtual ~Unpacker() noexcept {}

public:
    virtual const std::string& original_object() const final { return _obj; }
    virtual Unpacker& reset(const std::string& obj) final;

    template<typename T>
    /*virtual*/ Unpacker& unpack(std::list<T>& v, const std::string& name) throw(PackerError) /*final*/
    {
        v.clear();

        uint32_t size;
        unpack(size, name + "_count");
        for(uint32_t i=0; i<size; ++i) {
            T item;
            unpack(item, name);
            v.push_back(item);
        }
        return *this;
    }

    template<typename T>
    /*virtual*/ Unpacker& unpack(std::vector<T>& v, const std::string& name) throw(PackerError) /*final*/
    {
        v.clear();

        uint32_t size;
        unpack(size, name + "_count");
        for(uint32_t i=0; i<size; ++i) {
            T item;
            unpack(item, name);
            v.push_back(item);
        }
        return *this;
    }

    template<typename T>
    /*virtual*/ Unpacker& unpack(std::deque<T>& v, const std::string& name) throw(PackerError) /*final*/
    {
        v.clear();

        uint32_t size;
        unpack(size, name + "_count");
        for(uint32_t i=0; i<size; ++i) {
            T item;
            unpack(item, name);
            v.push_back(item);
        }
        return *this;
    }

#if 0
    template<typename K, typename V>
    /*virtual*/ Unpacker& unpack(std::map<K, V>& v, const std::string& name) throw(PackerError) /*final*/
    {
        v.clear();

        uint32_t size;
        unpack(size, name + "_count");
        for(uint32_t i=0; i<size; ++i) {
            K key;
            V value;

            unpack(key, name + "_key");
            unpack(value, name + "_value");
            v[key] = value;
        }
        return *this;
    }

    template<typename K, typename V>
    /*virtual*/ Unpacker& unpack(std::unordered_map<K, V>& v, const std::string& name) throw(PackerError) /*final*/
    {
        v.clear();

        uint32_t size;
        unpack(size, name + "_count");
        for(uint32_t i=0; i<size; ++i) {
            K key;
            V value;

            unpack(key, name + "_key");
            unpack(value, name + "_value");
            v[key] = value;
        }
        return *this;
    }
#endif

    template<typename T>
    /*virtual*/ Unpacker& unpack(std::shared_ptr<T>& t, const std::string& name) throw(PackerError) /*final*/
    {
        if(!t) { t.reset(new T()); }
        unpack(*t, name);
        return *this;
    }

    virtual Unpacker& unpack(Serializable& v, const std::string& name) throw(PackerError) final;

public:
    virtual unsigned int position() /*const*/ = 0;
    virtual Unpacker& skip(unsigned int count) throw(PackerError) = 0;
    virtual Unpacker& position(int unsigned position) throw(PackerError) = 0;
    virtual Unpacker& unpack(std::string& value, const std::string& name) throw(PackerError) = 0;
    virtual Unpacker& unpack(int8_t& value, const std::string& name) throw(PackerError) = 0;
    virtual Unpacker& unpack(uint8_t& value, const std::string& name) throw(PackerError) = 0;
    /*virtual Unpacker& unpack(int16_t& value, const std::string& name) throw(PackerError) = 0;
    virtual Unpacker& unpack(uint16_t& value, const std::string& name) throw(PackerError) = 0;*/
    virtual Unpacker& unpack(int32_t& value, const std::string& name) throw(PackerError) = 0;
    virtual Unpacker& unpack(uint32_t& value, const std::string& name) throw(PackerError) = 0;
    virtual Unpacker& unpack(int64_t& value, const std::string& name) throw(PackerError) = 0;
    virtual Unpacker& unpack(uint64_t& value, const std::string& name) throw(PackerError) = 0;
    virtual Unpacker& unpack(float& value, const std::string& name) throw(PackerError) = 0;
    virtual Unpacker& unpack(double& value, const std::string& name) throw(PackerError) = 0;
    virtual Unpacker& unpack(bool& value, const std::string& name) throw(PackerError) = 0;
    virtual bool done() const = 0;

protected:
    virtual Unpacker& on_reset() = 0;

protected:
    std::string _obj;

private:
    Unpacker();
    DISALLOW_COPY_AND_ASSIGN(Unpacker);
};

}

#endif
