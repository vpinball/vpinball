package org.vpinball.app.ui.screens.landing

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import org.vpinball.app.TableListMode
import org.vpinball.app.TableListSortOrder
import org.vpinball.app.VPinballManager
import org.vpinball.app.data.entity.PinTable
import org.vpinball.app.data.repository.PinTableRepository
import org.vpinball.app.jni.VPinballSettingsSection.STANDALONE

class LandingScreenViewModel(private val repository: PinTableRepository) : ViewModel() {
    private var tableJob: Job? = null

    private val _unfilteredTables = MutableStateFlow(emptyList<PinTable>())
    val unfilteredTables: StateFlow<List<PinTable>> = _unfilteredTables

    private val _filteredTables = MutableStateFlow(emptyList<PinTable>())
    val filteredTables: StateFlow<List<PinTable>> = _filteredTables

    private val _tableListMode = MutableStateFlow(TableListMode.TWO_COLUMN)
    val tableListMode: StateFlow<TableListMode> = _tableListMode

    private val _tableListSortOrder = MutableStateFlow(TableListSortOrder.A_Z)
    val tableListSortOrder: StateFlow<TableListSortOrder> = _tableListSortOrder

    private val _search = MutableStateFlow("")
    val search: StateFlow<String> = _search

    init {
        loadSettings()
        fetchTables()
    }

    fun setTableListMode(mode: TableListMode) {
        _tableListMode.update { mode }
        VPinballManager.saveValue(STANDALONE, "TableListMode", mode.value)
    }

    fun setTableSortOrder(order: TableListSortOrder) {
        _tableListSortOrder.update { order }
        VPinballManager.saveValue(STANDALONE, "TableListSort", order.value)
        fetchTables()
    }

    fun search(query: String) {
        _search.update { query }
        val unfilteredTables = _unfilteredTables.value
        if (query.isEmpty() || query.isBlank()) {
            _filteredTables.update { unfilteredTables.toList() }
        } else {
            _filteredTables.update { unfilteredTables.filter { it.name.lowercase().contains(query.trim().lowercase()) } }
        }
    }

    private fun fetchTables() {
        tableJob?.cancel()
        tableJob =
            viewModelScope.launch {
                repository.getAllSorted(isAscending = (_tableListSortOrder.value == TableListSortOrder.A_Z)).collect { sortedTables ->
                    _filteredTables.update { sortedTables }
                    _unfilteredTables.update { sortedTables }
                    search(search.value)
                }
            }
    }

    private fun loadSettings() {
        _tableListMode.value = TableListMode.fromInt(VPinballManager.loadValue(STANDALONE, "TableListMode", TableListMode.TWO_COLUMN.value))
        _tableListSortOrder.value = TableListSortOrder.fromInt(VPinballManager.loadValue(STANDALONE, "TableListSort", TableListSortOrder.A_Z.value))
    }
}
