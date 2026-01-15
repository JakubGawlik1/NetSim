#include "storage_types.hxx"
#include <stdexcept>





PackageQueue::PackageQueue(PackageQueueType type): queue_type_(type) {}

void PackageQueue::push(Package &&package) {
    container_.push_back(std::move(package));
}

//sprawdzanie czy lista jest pusta
bool PackageQueue::empty() const {
    return container_.empty();
}

//zwraca rozmiar
PackageQueue::size_type PackageQueue::size() const {
    return container_.size();
}

PackageQueue::const_iterator PackageQueue::begin() const {
    return container_.begin();
}

PackageQueue::const_iterator PackageQueue::end() const {
    return container_.end();
}

PackageQueue::const_iterator PackageQueue::cbegin() const {
    return container_.cbegin();
}

PackageQueue::const_iterator PackageQueue::cend() const {
    return container_.cend();
}

Package PackageQueue::pop() {
    if (container_.empty()) {
        throw std::runtime_error("Queue is empty");
    }
    else {
        if (queue_type_ == PackageQueueType::FIFO) {
            Package result = std::move(container_.front());
            container_.pop_front();
            return result;
        }
        else {
            Package result = std::move(container_.back());
            container_.pop_back();
            return result;
        }
    }
}



PackageQueueType PackageQueue::get_queue_type() const {
    return queue_type_;
}



