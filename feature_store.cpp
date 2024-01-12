#include "feature_store.h"

void IdentifiedFeatures::Append(donde_toolkits::Feature feature) {
    this->features.push_back(feature);
}

FeatureStore::FeatureStore() {}
FeatureStore::~FeatureStore(){};

void FeatureStore::AddFeature(const donde_toolkits::Feature& feature,
                              const std::string& identifier) {
    auto it = this->identified_features.find(identifier);
    if (it == this->identified_features.end()) {
        IdentifiedFeatures features{identifier};
        features.Append(feature);
        this->identified_features.insert({identifier, features});
    }

    this->identified_features.at(identifier).Append(feature);
}

void FeatureStore::DeleteFeature(const donde_toolkits::Feature& feature) {
    auto it = this->identified_features.find("");
    if (it != this->identified_features.end()) {
        it->second.Remove(feature.id);
    }
}

void FeatureStore::SearchFeature(const donde_toolkits::Feature& feature, int topK){};
