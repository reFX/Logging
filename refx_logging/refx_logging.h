
/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.txt file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:					refx_logging
  vendor:				reFX
  version:				1.0.0
  name:					reFX JUCE logging classes
  description:			Classes for easy logging/debugging output
  license:				none/internal use only
  minimumCppStandard:	11

  dependencies:     juce_core, juce_events, juce_graphics, juce_gui_basics, refx_gui_extras, refx_utilities

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/

#pragma once
#define REFX_DEBUGGING_H_INCLUDED

#include <optional>

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <refx_gui_extras/refx_gui_extras.h>

//==============================================================================

#include "Source/refx_logging.h"
#include "Source/refx_loggingWindow.h"
