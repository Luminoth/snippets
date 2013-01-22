#if !defined __RANDOM_H__
#define __RANDOM_H__

#if defined USE_OPENSSL
#include <openssl/bn.h>
#endif
#include <boost/random.hpp>

namespace energonsoftware {
namespace Random {

inline void seed(unsigned int s=std::time(nullptr))
{
    std::srand(s);
}

inline float base()
{
    static boost::mt19937 rng;
    static boost::uniform_real<float> range(0.0f,1.0f);
    static boost::variate_generator<boost::mt19937, boost::uniform_real<float>> gen(rng, range);
    return gen();
}

template <class T>
T uniform(T start, T end)
{
    return start + (T)((end - start) * base());
}

template <class T>
T& choice(std::vector<T>& l)
{
    return l[uniform<unsigned>(0, l.size())];
}

template <class T>
T& choice(std::deque<T>& l)
{
    return l[uniform<unsigned>(0, l.size())];
}

#if defined USE_OPENSSL
inline unsigned long large_rand_range(unsigned long range)
{
    BIGNUM* r = BN_new();
    BN_set_word(r, range);

    BIGNUM* v = BN_new();
    BN_rand_range(v, r);

    unsigned long ret = BN_get_word(v);

    BN_clear_free(r);
    BN_clear_free(v);
    return ret;
}
#endif

}
}

#endif
