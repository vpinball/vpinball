package org.vpinball.app

import org.vpinball.app.data.entity.PinTable

data class VPinballUiState(
    val splash: Boolean = true,
    val loading: Boolean = false,
    var table: PinTable? = null,
    var title: String? = null,
    var progress: Int = 0,
    var status: String? = null,
    var error: String? = null,
    val playing: Boolean = false,
    var touchInstructions: Boolean = false,
    var touchOverlay: Boolean = false,
    var liveUI: Boolean = false,
)
