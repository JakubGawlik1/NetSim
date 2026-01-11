#include "package.hxx"

// pola statyczne z hpp dizielone miedzy obiektami
std::set<ElementID> Package::assigned_IDs{};
std::set<ElementID> Package::freed_IDs{};

Package::Package(): id_(0) {
    // jesli nie jest puste to wez najmniejsze dostepne id (set jest posortowany)
    if (!freed_IDs.empty()) {
        auto it = freed_IDs.begin();
        id_ = *it;
        freed_IDs.erase(it);
    }
    else {
        //pierwsza paczka dostaje id =1
        if (assigned_IDs.empty()) {
            id_ = 1;
        }
        //każda kolejna dostaje id o 1 wieksze od najwiekszego id w assigned_Ids
        else {
            id_ = *assigned_IDs.rbegin() +1;
        }
    }
    assigned_IDs.insert(id_);
}

//tworzenie produktu z danym id
Package::Package(ElementID id): id_(id) {
    assigned_IDs.insert(id);//dodanie id jako zajętego
}


Package::Package(Package&& other) noexcept : id_(other.id_) {
    other.id_ = 0;
}

Package& Package::operator=(Package&& other) noexcept {
    if (this != &other) {
        if (id_ != 0) {
            assigned_IDs.erase(id_);
            freed_IDs.insert(id_);
        }
        id_ = other.id_;
        other.id_ = 0;
    }
    return *this;
}

//gry polprodukt zostanie usunięty, zwalnia się jego id
Package::~Package() {
    if (id_ != 0) {
        assigned_IDs.erase(id_);
        freed_IDs.insert(id_);
    }
}


ElementID Package::get_id() const {
    return id_;
}

