#pragma once

#include <list>
#include <memory>
#include "package.hxx"
#include "types.hxx"

//Package to polprodukt z ID

//typ kolejki
enum class PackageQueueType {
    FIFO, //pierwsza ktora przyszla (bierz z dolu)
    LIFO //ostatnia ktora przyszla (bierz z gory)
};

class IPackageStockpile {
    using const_iterator = std::list<Package>::const_iterator;

    // virtual const_iterator begin() const = 0;
    // virtual const_iterator end() const = 0;
    // virtual const_iterator cbegin() const = 0;
    // virtual const_iterator cend() const = 0;
    //
    // virtual ~IPackageStockpile() = default;
};

class IPackageQueue : public IPackageStockpile {

};


//
class PackageQueue: public IPackageQueue {

};
