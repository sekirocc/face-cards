#pragma once

#include "donde/definitions.h"

#include <string>
#include <unordered_map>

class IdentifiedFeatures {
  public:
    IdentifiedFeatures(std::string identifier) : _identifier{identifier} {}
    void Append(donde_toolkits::Feature feature);

  private:
    std::string _identifier;
    std::vector<donde_toolkits::Feature> features;
};

class FeatureStore {

  public:
    FeatureStore();
    ~FeatureStore();

    void AddFeature(const donde_toolkits::Feature& feature, const std::string& identifier);

    void DeleteFeature(const donde_toolkits::Feature& feature);

    void SearchFeature(const donde_toolkits::Feature& feature, int topK);

  private:
    std::unordered_map<std::string, IdentifiedFeatures> identified_features;
};
