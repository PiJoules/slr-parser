#include "lang.h"

/**
 * Borrowed from python3.6's tuple hash
 */
std::size_t lang::ProdRuleHasher::operator()(const ParseRule& prod_rule) const {
    std::string rule = prod_rule.rule;
    production_t prod = prod_rule.production;
    std::hash<std::string> str_hasher;
    std::size_t rule_hash = str_hasher(rule);

    std::size_t hash_mult = _HASH_MULTIPLIER;
    std::size_t prod_hash = 0x345678;
    std::size_t len = prod.size();
    for (std::string& r : prod){
        prod_hash = (prod_hash ^ str_hasher(r)) * hash_mult;
        hash_mult += 82520 + len + len;
    }
    prod_hash += 97531;
    return prod_hash ^ rule_hash;
}

std::size_t lang::ItemHasher::operator()(const lang::lr_item_t& lr_item) const {
    ProdRuleHasher hasher;
    return hasher(lr_item.first) ^ static_cast<std::size_t>(lr_item.second);
}

/**
 * Borrowed from python3.6's frozenset hash
 */ 
static std::size_t _shuffle_bits(std::size_t h){
    return ((h ^ 89869747UL) ^ (h << 16)) * 3644798167UL;
}

std::size_t lang::ItemSetHasher::operator()(const lang::item_set_t& item_set) const {
    std::size_t hash = 0;
    ItemHasher item_hasher;
    for (const auto& lr_item : item_set){
        hash ^= _shuffle_bits(item_hasher(lr_item));  // entry hashes
    }
    hash ^= item_set.size() * 1927868237UL;  // # of active entrues
    hash = hash * 69069U + 907133923UL;
    return hash;
};
