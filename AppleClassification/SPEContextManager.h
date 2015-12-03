#pragma once
#include <libspe2.h>

class SPEContextManager
{
public:
  SPEContextManager();
  bool initialise();
  spe_program_handle_t* getSPEImage(const char *filename);
  bool closeSPEImage(spe_program_handle_t *handle);
  spe_context_ptr_t createContext(unsigned int flags = 0);
  bool destroyContext(spe_context_ptr_t context);
  bool loadProgramHandle(spe_context_ptr_t target, spe_program_handle_t *handle);
  bool runSPEContext(spe_context_ptr_t, spe_stop_info*);
  bool runSPEContext(spe_context_ptr_t, spe_stop_info*, unsigned int *entry, unsigned int runflags, void *argp, void *envp);

  ~SPEContextManager();

private:
  int m_numPhysicalNodes, m_numPhysicalSPEs, m_numUsableSPEs;
};

