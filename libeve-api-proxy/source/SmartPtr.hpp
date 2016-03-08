#pragma once

#ifdef _WIN32

struct LocalFreeDeleter
{
    void operator()(wchar_t *s)const
    {
        LocalFree(s);
    }
};

#endif


template<class T>
class unique_out_ptr_wrapper
{
public:
    typedef typename T::pointer pointer;
    explicit unique_out_ptr_wrapper(T &uptr) : uptr(&uptr), p(nullptr) {}
    unique_out_ptr_wrapper(unique_out_ptr_wrapper<T> &&mv)
        : uptr(mv.uptr), p(mv.p)
    {
        mv.uptr = nullptr;
        mv.p = nullptr;
    }
    ~unique_out_ptr_wrapper()
    {
        if (uptr) uptr->reset(p);
    }
    operator pointer*() { return &p; }
private:
    T *uptr;
    pointer p;

    unique_out_ptr_wrapper(const unique_out_ptr_wrapper&) = delete;
    unique_out_ptr_wrapper& operator = (const unique_out_ptr_wrapper&) = delete;
    unique_out_ptr_wrapper& operator = (unique_out_ptr_wrapper&&) = delete;
};

template<class T> unique_out_ptr_wrapper<T> unique_out_ptr(T &uptr)
{
    return unique_out_ptr_wrapper<T>(uptr);
}
