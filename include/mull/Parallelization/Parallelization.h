#pragma once

#include "mull/Logger.h"
#include "mull/Parallelization/Progress.h"
#include "mull/Parallelization/TaskExecutor.h"

#include "mull/Parallelization/Tasks/ApplyMutationTask.h"
#include "mull/Parallelization/Tasks/BitcodeLoadingTask.h"
#include "mull/Parallelization/Tasks/DryRunMutantExecutionTask.h"
#include "mull/Parallelization/Tasks/InstrumentedCompilationTask.h"
#include "mull/Parallelization/Tasks/JunkDetectionTask.h"
#include "mull/Parallelization/Tasks/LoadObjectFilesTask.h"
#include "mull/Parallelization/Tasks/MutantExecutionTask.h"
#include "mull/Parallelization/Tasks/OriginalCompilationTask.h"
#include "mull/Parallelization/Tasks/OriginalTestExecutionTask.h"
#include "mull/Parallelization/Tasks/SearchMutationPointsTask.h"
