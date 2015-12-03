#include "SPEContextManager.h"
#include <iostream>


SPEContextManager::SPEContextManager() : m_numPhysicalNodes(-1), m_numPhysicalSPEs(-1), m_numUsableSPEs(-1)
{
}

bool SPEContextManager::initialise()
{
  m_numPhysicalNodes = spe_cpu_info_get(SPE_COUNT_PHYSICAL_CPU_NODES, 0);
  m_numPhysicalSPEs = spe_cpu_info_get(SPE_COUNT_PHYSICAL_SPES, 0);
  m_numUsableSPEs = spe_cpu_info_get(SPE_COUNT_USABLE_SPES, 0);

  return(m_numPhysicalNodes != -1 && m_numPhysicalSPEs != -1 && m_numUsableSPEs != -1);
}

spe_program_handle_t * SPEContextManager::getSPEImage(const char * filename)
{
  spe_program_handle_t *handle = spe_image_open(filename);

  if (handle == NULL)
  {
    std::cerr << "Failed to open SPE executable " << filename << std::endl;
  }

  return handle;
}

bool SPEContextManager::closeSPEImage(spe_program_handle_t * handle)
{
  if (handle)
  {
    if (spe_image_close(handle) == 0)
    {
      return true;
    }
  }

  return false;
}


spe_context * SPEContextManager::createContext(unsigned int flags)
{
  return spe_context_create(flags, NULL);
}

bool SPEContextManager::destroyContext(spe_context * context)
{
  if (context)
  {
    if (spe_context_destroy(context) == 0)
    {
      return true;
    }
  }

  return false;
}

bool SPEContextManager::loadProgramHandle(spe_context* target, spe_program_handle_t * handle)
{
  if (handle)
  {
    if (spe_program_load(target, handle) == 0)
    {
      return true;
    }
  }
  return false;
}

bool SPEContextManager::runSPEContext(spe_context_ptr_t context, spe_stop_info *stopInfo)
{
  return  runSPEContext(context, stopInfo, NULL);
}

bool SPEContextManager::runSPEContext(spe_context_ptr_t context, spe_stop_info* stopInfo, void *argp)
{
  unsigned int entry = SPE_DEFAULT_ENTRY;
  return  runSPEContext(context, stopInfo, &entry, 0, argp, NULL);
}

bool SPEContextManager::runSPEContext(spe_context_ptr_t context, spe_stop_info *stopinfo, unsigned int * entry, unsigned int runflags, void * argp, void * envp)
{
  if (spe_context_run(context, entry, runflags, argp, envp, stopinfo) == 0)
  {
    return true;
  }

  return false;
}


SPEContextManager::~SPEContextManager()
{
}
