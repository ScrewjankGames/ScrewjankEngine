module;
#include <string_view>
export module sj.editor:IPanel;

export namespace sj
{
class IPanel
{
public:
    virtual ~IPanel() = default;

    virtual void Draw() = 0;
    [[nodiscard]] virtual std::string_view GetName() const = 0;

private:
};
} // namespace sj