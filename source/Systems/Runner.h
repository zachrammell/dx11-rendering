#pragma once

#include <vector>
#include <memory>

#include "NonCopyable.h"

namespace IE::Systems
{

class IModule;
class RunnerBuilder;

class Runner
{
public:
  void Run();
  void Step();
  void Stop();
  ~Runner() = default;
  bool IsRunning();
public:
  static RunnerBuilder Create();
private:
  friend RunnerBuilder;
  Runner();
  std::vector<std::shared_ptr<IModule>> modules_;
  std::vector<int> system_indices_;
  bool is_running_;
};

class RunnerBuilder : NonCopyable
{
public:
  RunnerBuilder& AddModule(std::shared_ptr<IModule> module);
  Runner Initialize();
private:
  friend Runner;
  RunnerBuilder() = default;
  Runner runner_;
};

}
