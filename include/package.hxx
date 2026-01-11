#pragma once
#include "types.hxx"
#include <set>

class Package {

    public:
    Package();
    explicit Package(ElementID id);

    Package(Package&&) noexcept;
    Package& operator=(Package&&) noexcept;

    Package(const Package&) = delete;
    Package& operator=(const Package&) = delete;

    ElementID get_id() const;

    private:
    ElementID id_;
    static std::set<ElementID> assigned_IDs; // zajęte id
    static std::set<ElementID> freed_IDs; // wolne id

};
