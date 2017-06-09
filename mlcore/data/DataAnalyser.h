#ifndef DATAANALYSER_H_
#define DATAANALYSER_H_

#include <map>
#include <memory>
#include <vector>

#include "DataAnalyser.h"
#include "DataSource.h"
#include "Variable.h"
#include "VariableDomain.h"

namespace mlcore
{
  class VariableTypeDetector : public VariableDomain
  {
  public:
    virtual ~VariableTypeDetector()
    {}

    virtual bool match(const std::string& str) = 0;
  };

  typedef std::shared_ptr<VariableTypeDetector> VariableTypeDetectorPtr;
  typedef std::vector<VariableTypeDetectorPtr> VariableTypeDetectors;

  class UnclassifiedDetector : public VariableTypeDetector
  {
  public:
    virtual ~UnclassifiedDetector();
    virtual bool match(const std::string& str) override;
  };

  class NumberTypeDetector : public VariableTypeDetector
  {
  public:
    virtual ~NumberTypeDetector();
    virtual bool match(const std::string& str) override;
  };

  class DateTypeDetector : public VariableTypeDetector
  {
  public:
    virtual ~DateTypeDetector();
    virtual bool match(const std::string& str) override;
  };

  class VariableInfo
  {
  public:
    Variable variable;
    VariableTypeDetectors detectors;

  public:
    VariableInfo(const std::string& name, std::size_t num);
    std::size_t size() const;
    const VariableDomain& unclassified() const;
  };

  typedef std::vector<VariableInfo> VariableInfos;

  class DataAnalyser
  {
  public:
    DataAnalyser();
    void analyse(DataSource& data_source);
    const VariableInfos& variable_infos() const;

  private:
    VariableInfos info_;
  };
}

#endif /* DATAANALYSER_H_ */
