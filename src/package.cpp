#include "package.hxx"

Package::Package(): id_(0) {}
Package::Package(ElementID id): id_(id){}


ElementID Package::get_id() const {
    return id_;
}

