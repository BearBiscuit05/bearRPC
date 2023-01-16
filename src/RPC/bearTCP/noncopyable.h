
namespace bear
{
class noncopyable
{
private:
    noncopyable(const noncopyable&) = delete;
    void operator=(const noncopyable&) = delete;
public:
    noncopyable(/* args */) = default;
    ~noncopyable() = default;
};
}