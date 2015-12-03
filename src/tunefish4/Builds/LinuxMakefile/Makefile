# Automatically generated makefile, created by the Introjucer
# Don't edit this file! Your changes will be overwritten when you re-save the Introjucer project!

# (this disables dependency generation if multiple architectures are set)
DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)

ifndef CONFIG
  CONFIG=Debug
endif

ifeq ($(CONFIG),Debug)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Debug
  OUTDIR := build

  ifeq ($(TARGET_ARCH),)
    TARGET_ARCH := -march=native
  endif

  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCER_LINUX_MAKE_6D53C8B4=1" -D "JUCE_APP_VERSION=4.1.0" -D "JUCE_APP_VERSION_HEX=0x40100" -I /usr/include -I /usr/include/freetype2 -I ~/src/3rdparty/vst -I ../../JuceLibraryCode -I ../../../../../../3rdparty/GrapefruitJUCE/modules
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -g -ggdb -fPIC -O0
  CXXFLAGS += $(CFLAGS) -std=c++11
  LDFLAGS += $(TARGET_ARCH) -L$(BINDIR) -L$(LIBDIR) -Wl,--no-undefined -shared -L/usr/X11R6/lib/ -lGL -lX11 -lXext -lXinerama -lasound -ldl -lfreetype -lpthread -lrt 

  TARGET := Tunefish4.so
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
  CLEANCMD = rm -rf $(OUTDIR)/$(TARGET) $(OBJDIR)
endif

ifeq ($(CONFIG),Release)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Release
  OUTDIR := build

  ifeq ($(TARGET_ARCH),)
    TARGET_ARCH := -march=native
  endif

  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "NDEBUG=1" -D "JUCER_LINUX_MAKE_6D53C8B4=1" -D "JUCE_APP_VERSION=4.1.0" -D "JUCE_APP_VERSION_HEX=0x40100" -I /usr/include -I /usr/include/freetype2 -I ~/src/3rdparty/vst -I ../../JuceLibraryCode -I ../../../../../../3rdparty/GrapefruitJUCE/modules
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -fPIC -O3
  CXXFLAGS += $(CFLAGS) -std=c++11
  LDFLAGS += $(TARGET_ARCH) -L$(BINDIR) -L$(LIBDIR) -Wl,--no-undefined -shared -fvisibility=hidden -L/usr/X11R6/lib/ -lGL -lX11 -lXext -lXinerama -lasound -ldl -lfreetype -lpthread -lrt 

  TARGET := Tunefish4.so
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
  CLEANCMD = rm -rf $(OUTDIR)/$(TARGET) $(OBJDIR)
endif

OBJECTS := \
  $(OBJDIR)/tfsynthprogram_d4691da3.o \
  $(OBJDIR)/tflookandfeel_96d6a44d.o \
  $(OBJDIR)/PluginProcessor_a059e380.o \
  $(OBJDIR)/PluginEditor_94d4fb09.o \
  $(OBJDIR)/tf4_c53dd0b0.o \
  $(OBJDIR)/tf4fx_cda591c2.o \
  $(OBJDIR)/array_c34163e3.o \
  $(OBJDIR)/random_f6bd4543.o \
  $(OBJDIR)/runtime_3a4468a2.o \
  $(OBJDIR)/simd_a958b0ad.o \
  $(OBJDIR)/juce_audio_basics_fd261753.o \
  $(OBJDIR)/juce_audio_devices_d1a8e351.o \
  $(OBJDIR)/juce_audio_formats_2a2cf1.o \
  $(OBJDIR)/juce_audio_processors_b843ee0b.o \
  $(OBJDIR)/juce_core_6df5a635.o \
  $(OBJDIR)/juce_cryptography_feab1a8f.o \
  $(OBJDIR)/juce_data_structures_ca7ad431.o \
  $(OBJDIR)/juce_events_af2ff029.o \
  $(OBJDIR)/juce_graphics_879e930d.o \
  $(OBJDIR)/juce_gui_basics_9e57ac09.o \
  $(OBJDIR)/juce_gui_extra_3d974bd1.o \
  $(OBJDIR)/juce_opengl_6f781855.o \
  $(OBJDIR)/juce_PluginUtilities_afe645d.o \
  $(OBJDIR)/juce_video_530acff1.o \
  $(OBJDIR)/juce_VST_Wrapper_d13cec66.o \
  $(OBJDIR)/juce_VST3_Wrapper_7f4da424.o \

.PHONY: clean

$(OUTDIR)/$(TARGET): $(OBJECTS) $(RESOURCES)
	@echo Linking Tunefish4
	-@mkdir -p $(BINDIR)
	-@mkdir -p $(LIBDIR)
	-@mkdir -p $(OUTDIR)
	@$(BLDCMD)

clean:
	@echo Cleaning Tunefish4
	@$(CLEANCMD)

strip:
	@echo Stripping Tunefish4
	-@strip --strip-unneeded $(OUTDIR)/$(TARGET)

$(OBJDIR)/tfsynthprogram_d4691da3.o: ../../Source/tfsynthprogram.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling tfsynthprogram.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/tflookandfeel_96d6a44d.o: ../../Source/tflookandfeel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling tflookandfeel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PluginProcessor_a059e380.o: ../../Source/PluginProcessor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PluginProcessor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PluginEditor_94d4fb09.o: ../../Source/PluginEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PluginEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/tf4_c53dd0b0.o: ../../Source/synth/tf4.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling tf4.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/tf4fx_cda591c2.o: ../../Source/synth/tf4fx.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling tf4fx.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/array_c34163e3.o: ../../Source/runtime/array.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling array.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/random_f6bd4543.o: ../../Source/runtime/random.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling random.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/runtime_3a4468a2.o: ../../Source/runtime/runtime.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling runtime.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/simd_a958b0ad.o: ../../Source/runtime/simd.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling simd.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_audio_basics_fd261753.o: ../../../../../../3rdparty/GrapefruitJUCE/modules/juce_audio_basics/juce_audio_basics.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_audio_basics.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_audio_devices_d1a8e351.o: ../../../../../../3rdparty/GrapefruitJUCE/modules/juce_audio_devices/juce_audio_devices.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_audio_devices.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_audio_formats_2a2cf1.o: ../../../../../../3rdparty/GrapefruitJUCE/modules/juce_audio_formats/juce_audio_formats.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_audio_formats.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_audio_processors_b843ee0b.o: ../../../../../../3rdparty/GrapefruitJUCE/modules/juce_audio_processors/juce_audio_processors.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_audio_processors.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_core_6df5a635.o: ../../../../../../3rdparty/GrapefruitJUCE/modules/juce_core/juce_core.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_core.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_cryptography_feab1a8f.o: ../../../../../../3rdparty/GrapefruitJUCE/modules/juce_cryptography/juce_cryptography.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_cryptography.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_data_structures_ca7ad431.o: ../../../../../../3rdparty/GrapefruitJUCE/modules/juce_data_structures/juce_data_structures.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_data_structures.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_events_af2ff029.o: ../../../../../../3rdparty/GrapefruitJUCE/modules/juce_events/juce_events.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_events.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_graphics_879e930d.o: ../../../../../../3rdparty/GrapefruitJUCE/modules/juce_graphics/juce_graphics.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_graphics.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_gui_basics_9e57ac09.o: ../../../../../../3rdparty/GrapefruitJUCE/modules/juce_gui_basics/juce_gui_basics.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_gui_basics.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_gui_extra_3d974bd1.o: ../../../../../../3rdparty/GrapefruitJUCE/modules/juce_gui_extra/juce_gui_extra.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_gui_extra.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_opengl_6f781855.o: ../../../../../../3rdparty/GrapefruitJUCE/modules/juce_opengl/juce_opengl.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_opengl.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PluginUtilities_afe645d.o: ../../../../../../3rdparty/GrapefruitJUCE/modules/juce_audio_plugin_client/utility/juce_PluginUtilities.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_PluginUtilities.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_video_530acff1.o: ../../../../../../3rdparty/GrapefruitJUCE/modules/juce_video/juce_video.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_video.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_VST_Wrapper_d13cec66.o: ../../../../../../3rdparty/GrapefruitJUCE/modules/juce_audio_plugin_client/VST/juce_VST_Wrapper.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_VST_Wrapper.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_VST3_Wrapper_7f4da424.o: ../../../../../../3rdparty/GrapefruitJUCE/modules/juce_audio_plugin_client/VST3/juce_VST3_Wrapper.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_VST3_Wrapper.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

-include $(OBJECTS:%.o=%.d)
