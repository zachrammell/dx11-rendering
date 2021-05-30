#include "Runner.h"

#include "IModule.h"
#include "ISystem.h"

namespace IE::Systems
{

void Runner::Run()
{
  while (is_running_)
  {
    Step();
  }
}

void Runner::Step()
{
  for (int i : system_indices_)
  {
    std::dynamic_pointer_cast<ISystem>(modules_[i])->Update();
  }
}

void Runner::Stop()
{
  is_running_ = false;
}

Runner::Runner()
{
  is_running_ = true;
}

bool Runner::IsRunning()
{
  return is_running_;
}

RunnerBuilder Runner::Create()
{
  return RunnerBuilder();
}

RunnerBuilder& RunnerBuilder::AddModule(std::shared_ptr<IModule> module)
{
  runner_.modules_.emplace_back(std::move(module));
  return *this;
}

Runner RunnerBuilder::Initialize()
{
  int idx = 0;
  for (auto& module : runner_.modules_)
  {
    module->Initialize();
    if (module->IsSystem())
    {
      runner_.system_indices_.push_back(idx);
    }
    ++idx;
  }

  return runner_;
}

}
