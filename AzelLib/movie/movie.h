#pragma once

p_workArea overlayStart_MOVIE(p_workArea pWorkArea, s32 movieIndex);
p_workArea startTitleScreenVideo(p_workArea pWorkArea);
p_workArea loadMovieOverlay(p_workArea pWorkArea, s32 movieIndex);
void setupMovieVdp2();
void closeMovieStream();
u32 lastUpdateFunction();
