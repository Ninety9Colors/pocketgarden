#pragma once
#include <string>
#include <map>
#include <random>
#include <limits>

class Parameter {
public:
    Parameter();
    Parameter(const json& j);
    Parameter(float min, float value, float max);

    void seed_gaussian(std::mt19937_64& rng, float mean=std::numeric_limits<float>::infinity(), float stdev=std::numeric_limits<float>::infinity()) {
        mean = (mean == std::numeric_limits<float>::infinity()) ? (max+min)/2.0f : mean;
        stdev = (stdev == std::numeric_limits<float>::infinity()) ? (max-min)/6.0f : stdev;
        std::normal_distribution dist(mean,stdev);
        value = std::clamp<float>(dist(rng),min,max);
    }
    void seed_uniform(std::mt19937_64& rng) {
        std::uniform_real_distribution dist(min,max);
        value = std::clamp<float>(dist(rng),min,max);
    }
    void seed_log_normal(std::mt19937_64& rng, float mean=std::numeric_limits<float>::infinity(), float stdev=std::numeric_limits<float>::infinity(), float lower_bound=0.0f, float upper_bound=5.0f) {
        mean = (mean == std::numeric_limits<float>::infinity()) ? 0.0f : mean;
        stdev = (stdev == std::numeric_limits<float>::infinity()) ? 1.0f : stdev;
        std::lognormal_distribution<float> dist{mean,stdev};
        float value_one = (std::clamp<float>(dist(rng),lower_bound,upper_bound)-lower_bound)/(upper_bound-lower_bound);
        assert(value_one >= 0.0f && value_one <= 1.0f);
        value = (max-min)*value_one + min;
    }
    void seed_log_normal_inverse(std::mt19937_64& rng, float mean=std::numeric_limits<float>::infinity(), float stdev=std::numeric_limits<float>::infinity(), float lower_bound=0.0f, float upper_bound=5.0f) {
        mean = (mean == std::numeric_limits<float>::infinity()) ? 0.0f : mean;
        stdev = (stdev == std::numeric_limits<float>::infinity()) ? 1.0f : stdev;
        std::lognormal_distribution<float> dist{mean,stdev};
        float value_one = 1.0f-(std::clamp<float>(dist(rng),lower_bound,upper_bound)-lower_bound)/(upper_bound-lower_bound);
        assert(value_one >= 0.0f && value_one <= 1.0f);
        value = (max-min)*value_one + min;
    }

    json to_json() const;
    void from_json(const json& j);

    float min;
    float value;
    float max;
};

class ParameterMap {
public:
    ParameterMap();
    ParameterMap(const json& j);

    void set_parameter(std::string name, float value);
    void set_parameter(std::string name, Parameter parameter);
    const Parameter get_parameter(std::string name) const;
    bool contains_parameter(std::string name) const;

    void seed_gaussian(std::string name, std::mt19937_64& rng, float mean=std::numeric_limits<float>::infinity(), float stdev=std::numeric_limits<float>::infinity()) {
        parameters_.at(name).seed_gaussian(rng,mean,stdev);
    }
    void seed_uniform(std::string name, std::mt19937_64& rng) {
        parameters_.at(name).seed_uniform(rng);
    }
    void seed_log_normal(std::string name, std::mt19937_64& rng, float mean=std::numeric_limits<float>::infinity(), float stdev=std::numeric_limits<float>::infinity(), float lower_bound=0.0f, float upper_bound=5.0f) {
        parameters_.at(name).seed_log_normal(rng,mean,stdev,lower_bound,upper_bound);
    }
    void seed_log_normal_inverse(std::string name, std::mt19937_64& rng, float mean=std::numeric_limits<float>::infinity(), float stdev=std::numeric_limits<float>::infinity(), float lower_bound=0.0f, float upper_bound=5.0f) {
        parameters_.at(name).seed_log_normal_inverse(rng,mean,stdev,lower_bound,upper_bound);
    }

    json to_json() const;
    void from_json(const json& j);
private:
    std::map<std::string, Parameter> parameters_;
};