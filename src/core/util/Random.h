#if !defined __RANDOM_H__
#define __RANDOM_H__

#if defined USE_OPENSSL
    #include <openssl/bn.h>
#endif
#include <random>

namespace energonsoftware {

template<typename Generator=std::default_random_engine>
class Random
{
public:
    static void seed_std(unsigned int seed=std::time(nullptr))
    {
        assert(seed > 0);
        std::srand(seed);
    }

    template<typename T>
    static T uniform_std(T start=0, T end=1)
    {
        assert(std::is_arithmetic<T>::value);
        assert(start < end);

        return start + (end - start) * rand/*om*/();
    }

    static unsigned long large_rand_range(unsigned long range)
    {
#if defined USE_OPENSSL
        BIGNUM* r = BN_new();
        BN_set_word(r, range);

        BIGNUM* v = BN_new();
        BN_rand_range(v, r);

        unsigned long ret = BN_get_word(v);

        BN_clear_free(r);
        BN_clear_free(v);
        return ret;
#else
        // TODO: write non-OpenSSL version
        return 0;
#endif
    }

public:
    Random()
        : _engine()
    {
    }

    explicit Random(unsigned int seed)
        : _engine()
    {
        this->seed(seed);
    }

    virtual ~Random() noexcept
    {
    }

public:
    void seed(unsigned int seed=std::time(nullptr))
    {
        assert(seed > 0);
        _engine.seed(seed);
    }

    template<typename T>
    T uniform(T start=0, T end=1)
    {
        assert(std::is_arithmetic<T>::value);
        assert(start < end);

        typedef typename boost::mpl::if_c<std::is_floating_point<T>::value, std::uniform_real_distribution<T>, std::uniform_int_distribution<T>>::type Distribution;
        return Distribution(start, end)(_engine);
    }

    // TODO: both of these should take iterators maybe? is that possible?
    template<typename T>
    T& choice(const std::vector<T>& v)
    {
        return v[uniform<unsigned int>(0, v.size())];
    }

    template<typename T>
    T& choice(const std::deque<T>& v)
    {
        return v[uniform<unsigned int>(0, v.size())];
    }

private:
    Generator _engine;
};

}

#endif
