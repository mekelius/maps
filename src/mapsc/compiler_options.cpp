#include "compiler_options.hh"

#include <array>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

#include "compiler_option_defs.inc"


using std::optional, std::nullopt, std::unique_ptr, std::make_unique;

namespace Maps {

// Private static variables
std::map<std::jthread::id, CompilerOptions::Entries*> CompilerOptions::entries_per_thread;
optional<CompilerOptions::Entries*> CompilerOptions::global_entries_ = nullopt;

optional<unique_ptr<CompilerOptions>> CompilerOptions::lock(
    const Entries& entries, bool for_all_threads) {
    
    if (global_entries_)
        return nullopt;
    
    if (entries_per_thread.contains(std::this_thread::get_id()))
        return nullopt;

    return unique_ptr<CompilerOptions>(new CompilerOptions{entries, for_all_threads});
}

std::optional<std::unique_ptr<CompilerOptions>> CompilerOptions::lock_for_this_thread(
    const Entries& entries) {

    return lock(entries, false);
}

// private constructor
CompilerOptions::CompilerOptions(const Entries& entries, bool for_all_threads) {
    entries_ = make_unique<Entries>(entries);

    if (for_all_threads) {
        global_entries_ = entries_.get();
        return;
    }

    entries_per_thread.insert_or_assign(std::this_thread::get_id(), entries_.get());
}

CompilerOptions::~CompilerOptions() {
    if (global_entries_) {
        global_entries_ = nullopt;
        return;
    }

    entries_per_thread.erase(std::this_thread::get_id());
}

std::string_view CompilerOptions::get(CompilerOption key) {
    Entries* entries;

    if (global_entries_) {
        entries = *global_entries_;
    
    } else {
        auto thread_entries_it = entries_per_thread.find(std::this_thread::get_id());
        
        if (thread_entries_it == entries_per_thread.end())
            return get_default(key);

        entries = thread_entries_it->second;
    }

    auto entry_it = entries->find(key);

    if (entry_it == entries->end())
        return get_default(key);

    return entry_it->second;
}

CompilerOptions::Entry CompilerOptions::get_key_val_pair(CompilerOption key) {
    return {get_key_string(key), get(key)};
}

void CompilerOptions::set(CompilerOption key, const std::string& value) {
    entries_->insert_or_assign(key, value);
}

std::string_view CompilerOptions::get_key_string(CompilerOption key) {
    return default_entries.at(static_cast<int>(key)).first;
}

std::string_view CompilerOptions::get_default(CompilerOption key) {
    return default_entries.at(static_cast<int>(key)).second;
}   

} // namespace Maps