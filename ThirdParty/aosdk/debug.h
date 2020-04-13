/*
 * Audio Overload SDK
 *
 * Debug GUI
 *
 * Author: Nmlgc
 */

#ifdef __cplusplus

/// Widgets (C++ only)
/// -------
struct DebugMemoryState {
	int offset = 0;
	int rows = 16;
};
void debug_memory(const char *view_id, DebugMemoryState *state, uint8 *mem_buf, int32 mem_size);
/// -------

extern "C" {
#endif

typedef void debug_hw_t(void);

// All functions return false when debugging should be stopped.
ao_bool debug_start(void);
ao_bool debug_frame(debug_hw_t *hw);
ao_bool debug_stop(void);

#ifdef __cplusplus
}
#endif
