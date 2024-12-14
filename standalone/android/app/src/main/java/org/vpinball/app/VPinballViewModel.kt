package org.vpinball.app

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlin.time.Duration.Companion.milliseconds
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.firstOrNull
import kotlinx.coroutines.flow.flow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import kotlinx.datetime.Clock
import kotlinx.datetime.TimeZone
import kotlinx.datetime.toLocalDateTime
import org.vpinball.app.data.entity.PinTable
import org.vpinball.app.data.repository.PinTableRepository

class VPinballViewModel(private val repository: PinTableRepository) : ViewModel() {
    private val _state = MutableStateFlow(VPinballUiState())
    val state: StateFlow<VPinballUiState> = _state.asStateFlow()

    var webServerURL by mutableStateOf("")

    var progress = mutableIntStateOf(0)
        private set

    var status = mutableStateOf("")
        private set

    init {
        viewModelScope.launch {
            delay(SPLASH_DELAY_DURATION)
            _state.update { it.copy(splash = false) }
        }
    }

    fun title(value: String?) {
        _state.update { it.copy(title = value) }
    }

    fun progress(value: Int) {
        _state.update { it.copy(progress = value) }
        progress.intValue = value
    }

    fun status(value: String?) {
        _state.update { it.copy(status = value) }
        status.value = value ?: ""
    }

    fun loading(isLoading: Boolean, table: PinTable? = state.value.table) {
        _state.update { it.copy(loading = isLoading, table = table, title = table?.name ?: "", error = null) }
    }

    fun playing(isPlaying: Boolean) {
        _state.update { it.copy(playing = isPlaying) }
    }

    fun clearError() {
        _state.update { it.copy(error = null) }
    }

    fun setError(error: String) {
        _state.update { it.copy(error = error) }
    }

    fun stopped() {
        _state.update { it.copy(loading = false, playing = false, liveUI = false, table = null) }
    }

    fun toggleLiveUI() {
        _state.update { it.copy(liveUI = !it.liveUI) }
    }

    fun isLiveUI(): Boolean {
        return _state.value.liveUI
    }

    fun touchInstructions(isVisible: Boolean) {
        _state.update { it.copy(touchInstructions = isVisible) }
    }

    fun touchOverlay(isVisible: Boolean) {
        _state.update { it.copy(touchOverlay = isVisible) }
    }

    fun saveImportTable(uuid: String, path: String): Flow<PinTable> {
        return flow {
            val existingTable = repository.getById(uuid).firstOrNull()
            if (existingTable == null) {
                val name = path.substringBeforeLast('.').replace(Regex("[_]"), " ")
                val newTable = PinTable(uuid, name, path)
                repository.insert(newTable)
                emit(newTable)
            } else {
                emit(existingTable)
            }
        }
    }

    fun renameTable(table: PinTable, name: String): Flow<Boolean> {
        return flow {
            val result = runCatching {
                repository.update(table.copy(name = name, modifiedAt = Clock.System.now().toLocalDateTime(TimeZone.currentSystemDefault())))
            }
            emit(result.isSuccess)
            if (result.isFailure) {
                result.exceptionOrNull()?.printStackTrace()
            }
        }
    }

    fun markTableAsModified(table: PinTable): Flow<Boolean> {
        return flow {
            val result = runCatching {
                repository.update(table.copy(modifiedAt = Clock.System.now().toLocalDateTime(TimeZone.currentSystemDefault())))
            }
            emit(result.isSuccess)
            if (result.isFailure) {
                result.exceptionOrNull()?.printStackTrace()
            }
        }
    }

    fun deleteTable(table: PinTable): Flow<Boolean> {
        return flow {
            val result = runCatching { repository.delete(table) }
            emit(result.isSuccess)
            if (result.isFailure) {
                result.exceptionOrNull()?.printStackTrace()
            }
        }
    }

    companion object {
        private val SPLASH_DELAY_DURATION = 2000.milliseconds
    }
}
