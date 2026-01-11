#pragma once
#include "types.hxx"

class Package {

    public:
    Package();
    explicit Package(ElementID id);

    Package(Package&&) noexcept;
    Package& operator=(Package&&) noexcept;

    Package(const Package&) = delete;
    Package& operator=(const Package&) = delete;

    ElementID get_id() const;
    ~Package();

    private:
    ElementID id_;
};
