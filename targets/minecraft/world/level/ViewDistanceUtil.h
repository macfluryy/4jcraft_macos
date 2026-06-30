#pragma once

// Shared, pure view-distance helpers usable by BOTH client and server code.
//
// These functions are intentionally free of any state, globals or side
// effects so they can be linked into the client (rendering/options) path as
// well as the server (PlayerChunkMap / chunk streaming) path and always agree
// on how a view distance is interpreted and bounded.
//
// Include hygiene: this header deliberately pulls in NO server headers. The
// authoritative bounds live in PlayerChunkMap (MIN_VIEW_DISTANCE /
// MAX_VIEW_DISTANCE) and are used directly inside ViewDistanceUtil.cpp, so
// client translation units that only need these helpers do not transitively
// drag in server-side types.

// Clamp an arbitrary chunk view distance into the valid server range
// [PlayerChunkMap::MIN_VIEW_DISTANCE, PlayerChunkMap::MAX_VIEW_DISTANCE].
int clampViewDistance(int chunks);

// Map the client Options::viewDistance option (0..3, where 0 == Far) to a
// number of chunks, consistent with GameRenderer::setupCamera which uses
// renderDistance(blocks) = 16 * 16 >> viewDistance. Converted to chunks that
// is (16 >> viewDistance), i.e. 16, 8, 4, 2 chunks for options 0..3. The
// result is then passed through clampViewDistance.
int viewDistanceOptionToChunks(int optionVD);

// Compute the effective view distance applied to a player as the clamped
// minimum of the server limit and the client request:
// clampViewDistance(min(server, client)).
int effectiveViewDistance(int server, int client);
