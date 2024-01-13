#pragma once

#include "donde/definitions.h"
#include "donde/feature_search/feature_topk_rank.h"

#include <string>
#include <unordered_map>

class UserFeatures {
  public:
    UserFeatures(std::string identifier) : _identifier{identifier} {}
    void Append(const donde_toolkits::Feature& feature);
    int Remove(const donde_toolkits::Feature& feature);
    std::vector<donde_toolkits::Feature> Search(const donde_toolkits::Feature& feature, int topK);
    std::vector<donde_toolkits::Feature> Search(const donde_toolkits::Feature& feature, int topK, float min_score);

  private:
    std::size_t make_feature_key(const donde_toolkits::Feature& feature);

  private:
    std::string _identifier;
    std::unordered_map<std::size_t, donde_toolkits::Feature> features;
};

class FeatureStore {

  public:
    FeatureStore();
    ~FeatureStore();

    void AddFeature(const donde_toolkits::Feature& feature, const std::string& identifier);

    // DeleteFeature delete this feature from store, use equality compare, means similarity = 1,
    // it's a slow operation, rarely used.
    void DeleteFeature(const donde_toolkits::Feature& feature);

    std::vector<donde_toolkits::Feature>  SearchFeature(const donde_toolkits::Feature& feature, int topK);
    std::vector<donde_toolkits::Feature>  SearchFeature(const donde_toolkits::Feature& feature, int topK, float min_score);

  private:
    std::unordered_map<std::string, UserFeatures> identified_features;
};
