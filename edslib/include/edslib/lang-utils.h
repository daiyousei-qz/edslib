#pragma once

namespace eds
{
#define EDSLIB_DISABLE_COPY(KLASS_NAME)     \
    KLASS_NAME(const KLASS_NAME&) = delete; \
    KLASS_NAME& operator=(const KLASS_NAME&) = delete;

#define EDSLIB_DISABLE_MOVE(KLASS_NAME) \
    KLASS_NAME(KLASS_NAME&&) = delete;  \
    KLASS_NAME& operator=(KLASS_NAME&&) = delete;

#define EDSLIB_DISABLE_COPYMOVE(KLASS_NAME) \
    EDSLIB_DISABLE_COPY(KLASS_NAME)         \
    EDSLIB_DISABLE_MOVE(KLASS_NAME)

    class NonCopyable
    {
    protected:
        NonCopyable()                   = default;
        NonCopyable(const NonCopyable&) = delete;
        NonCopyable& operator=(const NonCopyable&) = delete;
    };

    class NonMovable
    {
    protected:
        NonMovable()             = default;
        NonMovable(NonMovable&&) = delete;
        NonMovable& operator=(NonMovable&&) = delete;
    };
} // namespace eds