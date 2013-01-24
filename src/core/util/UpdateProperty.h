#if !defined __UPDATEPROPERTY_H__
#define __UPDATEPROPERTY_H__

namespace energonsoftware {

class UpdatePropertyContainer
{
public:
    UpdatePropertyContainer() {}
    virtual ~UpdatePropertyContainer() throw() {}

public:
    virtual bool dirty() const final { return on_dirty() || db_dirty(); }
    virtual bool db_dirty() const final { return on_db_dirty(); }

public:
    // call super-class clean first when overriding
    virtual void clean() {}

protected:
    // call super-class _dirty first when overriding
    virtual bool on_dirty() const { return false; }

    // call super-class _db_dirty first when overriding
    virtual bool on_db_dirty() const { return false; }
};

template<typename T>
class UpdateProperty final
{
public:
    UpdateProperty()
    {
    }

    explicit UpdateProperty(const T& value, bool dirty=false)
        : _value(value), _dirty(dirty)
    {
    }

    virtual ~UpdateProperty() throw() {}

public:
    void clean() { _dirty = false; }
    bool dirty() const { return _dirty; }

    void value(const T& value)
    {
        _dirty = true;
        _value = value;
    }
    void operator=(const T& rhs) { value(rhs); }

    const T& value() const { return _value; }
    T& value() { return _value; }

private:
    T _value;
    bool _dirty;

private:
    DISALLOW_COPY_AND_ASSIGN(UpdateProperty);
};

}

#endif
