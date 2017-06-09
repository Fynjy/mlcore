#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Features.h"

namespace mlcore
{
  class VariableInfo;

  class FeatureMapper
  {
  public:
    virtual ~FeatureMapper()
    {}

    virtual double map(const std::string& str) = 0;
    virtual FeatureType feature_type() const = 0;
    virtual bool match(const VariableInfo& info) = 0;
  };

  typedef std::shared_ptr<FeatureMapper> FeatureMapperPtr;

  class NumberFeatureMapper : public FeatureMapper
  {
  public:
    NumberFeatureMapper();
    virtual ~NumberFeatureMapper();
    virtual double map(const std::string& str) override;
    virtual FeatureType feature_type() const override;
    virtual bool match(const VariableInfo& info) override;
  };

  class CategoryFeatureMapper : public FeatureMapper
  {
  public:
    virtual ~CategoryFeatureMapper();
    virtual double map(const std::string& str) override;
    virtual FeatureType feature_type() const override;
    virtual bool match(const VariableInfo& info) override;

  private:
    std::map<std::string, std::size_t> mapping_;
  };

  class DateFeatureMapper : public FeatureMapper
  {
  public:
    virtual ~DateFeatureMapper();
    virtual double map(const std::string& str) override;
    virtual FeatureType feature_type() const override;
    virtual bool match(const VariableInfo& info) override;
  };
}
