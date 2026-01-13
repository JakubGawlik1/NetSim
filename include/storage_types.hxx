#pragma once

#include <list>
#include "package.hxx"
#include <cstddef>

//Package to polprodukt z ID

//typ kolejki
enum class PackageQueueType {
    FIFO, //pierwsza ktora przyszla (bierz z dolu)
    LIFO //ostatnia ktora przyszla (bierz z gory)
};

//klasa abstrakcyjna
class IPackageStockpile {
public:
    using const_iterator = std::list<Package>::const_iterator;
    using size_type = std::size_t;

    virtual const_iterator begin() const = 0;
    virtual const_iterator end() const = 0;
    virtual const_iterator cbegin() const = 0;
    virtual const_iterator cend() const = 0;

    virtual ~IPackageStockpile() = default; //domyślny destruktor wirtualny

    virtual void push(Package&& package) = 0; //umieszcza produkt na skladowisku

    virtual size_type size() const = 0;//zwraca ilosc produktow w kontenerze
    virtual bool empty() const = 0; //sprawdzanie czy kontener jest pusty
};

//dodanie funkcjonalności usuwania obiektu i zwracania typu kolejki
class IPackageQueue : public IPackageStockpile {
public:
    virtual Package pop() = 0; //usuwa polprodukt z kolejki

    virtual PackageQueueType get_queue_type() const = 0; //zwraca typ kolejki (FIFO, LIFO)

    virtual ~IPackageQueue() = default;
};


//
class PackageQueue: public IPackageQueue {
public:

    explicit PackageQueue(PackageQueueType type);

    void push( Package&& package) override;
    bool empty() const override;
    size_type size() const override;


    const_iterator begin() const override;
    const_iterator end() const override;
    const_iterator cbegin() const override;
    const_iterator cend() const override;

    Package pop() override;
    PackageQueueType get_queue_type() const override;

private:
    std::list<Package> container_; //lista półproduktów
    PackageQueueType queue_type_; //typ kolejki
};

