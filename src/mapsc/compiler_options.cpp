#include "compiler_options.hh"

#include <array>

#include "default_option_defs.inc"


using std::optional, std::nullopt, std::unique_ptr, std::make_unique;

namespace Maps {

// Private static variables
std::map<std::jthread::id, CompilerOptions::Entries*> CompilerOptions::entries_per_thread;

optional<unique_ptr<CompilerOptions>> CompilerOptions::lock(const Entries& entries) {
    if (entries_per_thread.contains(std::this_thread::get_id()))
        return nullopt;

    return unique_ptr<CompilerOptions>(new CompilerOptions{entries});
}

// private constructor
CompilerOptions::CompilerOptions(const Entries& entries) {
    entries_ = make_unique<Entries>(entries);
    entries_per_thread.insert_or_assign(std::this_thread::get_id(), entries_.get());
}

CompilerOptions::~CompilerOptions() {
    entries_per_thread.erase(std::this_thread::get_id());
}

std::string_view CompilerOptions::get(CompilerOption key) {
    auto thread_entries_it = entries_per_thread.find(std::this_thread::get_id());
    
    if (thread_entries_it == entries_per_thread.end())
        return get_default(key);

    auto [_1, thread_entries] = *thread_entries_it;
    auto entry_it = thread_entries->find(key);

    if (entry_it == thread_entries->end())
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