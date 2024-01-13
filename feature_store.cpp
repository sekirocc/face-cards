#include "feature_store.h"

std::size_t UserFeatures::make_feature_key(const donde_toolkits::Feature& feature) {
    auto char_ptr = reinterpret_cast<const char*>(feature.raw.data());
    auto char_len = feature.raw.size() * sizeof(float);
    std::string_view sv{char_ptr, char_len};
    return std::hash<std::string_view>{}(sv);
}

void UserFeatures::Append(const donde_toolkits::Feature& feature) {
    std::size_t key = make_feature_key(feature);
    this->features.insert({key, feature});
}

int UserFeatures::Remove(const donde_toolkits::Feature& feature) {
    std::size_t key = make_feature_key(feature);
    return this->features.erase(key);
}

std::vector<donde_toolkits::Feature> UserFeatures::Search(const donde_toolkits::Feature& query,
                                                          int topK) {
    // default min_score is 0.8f
    return Search(query, topK, 0.8f);
}
std::vector<donde_toolkits::Feature>
UserFeatures::Search(const donde_toolkits::Feature& query, int topK, float min_score) {
    donde_toolkits::FeatureTopkRanking rank{query, topK};
    for (auto const& [_, ft] : this->features) {
        rank.FeedIn(ft);
    }
    auto results = rank.SortOut();

    std::vector<donde_toolkits::Feature> similar_features;
    similar_features.reserve(results.size());
    for (std::size_t i = 0; i < results.size(); ++i) {
        if (results[i].score < min_score) {
            continue;
        }
        similar_features.push_back(results[i].target);
    }

    return similar_features;
}

FeatureStore::FeatureStore() {}

FeatureStore::~FeatureStore() {}

void FeatureStore::AddFeature(const donde_toolkits::Feature& feature,
                              const std::string& identifier) {
    auto it = this->identified_features.find(identifier);
    if (it == this->identified_features.end()) {
        UserFeatures features{identifier};
        features.Append(feature);
        this->identified_features.insert({identifier, features});
    }
    this->identified_features.at(identifier).Append(feature);
}

void FeatureStore::DeleteFeature(const donde_toolkits::Feature& feature) {
    for (auto& [key, user_features] : identified_features) {
        int n = user_features.Remove(feature);
        spdlog::info("delete {} feature from user store: {}", n, key);
    }
}

std::vector<donde_toolkits::Feature>
FeatureStore::SearchFeature(const donde_toolkits::Feature& query, int topK) {
    return SearchFeature(query, topK, 0.8f);
}

std::vector<donde_toolkits::Feature>
FeatureStore::SearchFeature(const donde_toolkits::Feature& query, int topK, float min_score) {
    donde_toolkits::FeatureTopkRanking total_rank{query, topK};

    for (auto& [key, user_features] : identified_features) {
        auto fts = user_features.Search(query, topK, min_score);
        spdlog::info(
            "search {} results in user_features: {}, send them to total rank.", fts.size(), key);
        for (auto const& f : fts) {
            total_rank.FeedIn(f);
        }
    }

    auto results = total_rank.SortOut();
    std::vector<donde_toolkits::Feature> similar_features(results.size());
    for (std::size_t i = 0; i < results.size(); ++i) {
        similar_features[i] = results[i].target;
    }
    return similar_features;
}
