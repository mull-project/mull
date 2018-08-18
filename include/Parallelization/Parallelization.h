#pragma once

#include "Logger.h"
#include "Parallelization/Progress.h"
#include "Parallelization/TaskExecutor.h"

#include "Parallelization/Tasks/ModuleLoadingTask.h"
#include "Parallelization/Tasks/SearchMutationPointsTask.h"
#include "Parallelization/Tasks/LoadObjectFilesTask.h"
#include "Parallelization/Tasks/DryRunMutantExecutionTask.h"
#include "Parallelization/Tasks/InstrumentedCompilationTask.h"
