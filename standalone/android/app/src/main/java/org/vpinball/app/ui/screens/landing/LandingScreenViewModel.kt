package org.vpinball.app.ui.screens.landing

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.SharedFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asSharedFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import org.vpinball.app.Table
import org.vpinball.app.TableListMode
import org.vpinball.app.TableListSortOrder
import org.vpinball.app.TableManager
import org.vpinball.app.VPinballManager
import org.vpinball.app.jni.VPinballSettingsSection.STANDALONE

class LandingScreenViewModel : ViewModel() {
    companion object {
        private val _refreshTrigger = MutableSharedFlow<Unit>(extraBufferCapacity = 1)
        val refreshTrigger: SharedFlow<Unit> = _refreshTrigger.asSharedFlow()

        private val _scrollToTableTrigger = MutableSharedFlow<Table>(extraBufferCapacity = 1)
        val scrollToTableTrigger: SharedFlow<Table> = _scrollToTableTrigger.asSharedFlow()

        fun triggerRefresh() {
            _refreshTrigger.tryEmit(Unit)
        }

        fun triggerScrollToTable(table: Table) {
            _scrollToTableTrigger.tryEmit(table)
        }

        private val _updateTableTrigger = MutableSharedFlow<Table>(extraBufferCapacity = 1)
        val updateTableTrigger: SharedFlow<Table> = _updateTableTrigger.asSharedFlow()

        fun triggerUpdateTable(table: Table) {
            _updateTableTrigger.tryEmit(table)
        }

        private val _addTablesTrigger = MutableSharedFlow<List<Table>>(extraBufferCapacity = 1)
        val addTablesTrigger: SharedFlow<List<Table>> = _addTablesTrigger.asSharedFlow()

        fun triggerAddTables(tables: List<Table>) {
            _addTablesTrigger.tryEmit(tables)
        }

        private val _removeTableTrigger = MutableSharedFlow<Table>(extraBufferCapacity = 1)
        val removeTableTrigger: SharedFlow<Table> = _removeTableTrigger.asSharedFlow()

        fun triggerRemoveTable(table: Table) {
            _removeTableTrigger.tryEmit(table)
        }
    }

    private var tableJob: Job? = null

    private val _unfilteredTables = MutableStateFlow(emptyList<Table>())
    val unfilteredTables: StateFlow<List<Table>> = _unfilteredTables

    private val _filteredTables = MutableStateFlow(emptyList<Table>())
    val filteredTables: StateFlow<List<Table>> = _filteredTables

    private val _tableListMode = MutableStateFlow(TableListMode.TWO_COLUMN)
    val tableListMode: StateFlow<TableListMode> = _tableListMode

    private val _tableListSortOrder = MutableStateFlow(TableListSortOrder.A_Z)
    val tableListSortOrder: StateFlow<TableListSortOrder> = _tableListSortOrder

    private val _search = MutableStateFlow("")
    val search: StateFlow<String> = _search

    private val _isLoading = MutableStateFlow(false)
    val isLoading: StateFlow<Boolean> = _isLoading

    private val _loadingProgress = MutableStateFlow(0)
    val loadingProgress: StateFlow<Int> = _loadingProgress

    private val _loadingStatus = MutableStateFlow("")
    val loadingStatus: StateFlow<String> = _loadingStatus

    init {
        loadSettings()

        if (_unfilteredTables.value.isEmpty()) {
            fetchTables()
        }

        viewModelScope.launch { refreshTrigger.collect { refreshTables() } }

        viewModelScope.launch { updateTableTrigger.collect { table -> updateSingleTable(table) } }

        viewModelScope.launch { addTablesTrigger.collect { tables -> addNewTables(tables) } }

        viewModelScope.launch { removeTableTrigger.collect { table -> removeTable(table) } }
    }

    fun setTableListMode(mode: TableListMode) {
        _tableListMode.update { mode }
        VPinballManager.saveValue(STANDALONE, "TableListMode", mode.value)
    }

    fun setTableSortOrder(order: TableListSortOrder) {
        _tableListSortOrder.update { order }
        VPinballManager.saveValue(STANDALONE, "TableListSort", order.value)

        val sortedTables =
            when (order) {
                TableListSortOrder.A_Z -> _unfilteredTables.value.sortedBy { it.name }
                TableListSortOrder.Z_A -> _unfilteredTables.value.sortedByDescending { it.name }
            }

        _unfilteredTables.update { sortedTables }
        _filteredTables.update { sortedTables }
        search(search.value)
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

    fun refreshTables() {
        fetchTables()
    }

    private fun updateSingleTable(updatedTable: Table) {
        _unfilteredTables.update { tables -> tables.map { if (it.uuid == updatedTable.uuid) updatedTable else it } }
        _filteredTables.update { tables -> tables.map { if (it.uuid == updatedTable.uuid) updatedTable else it } }
    }

    private fun addNewTables(newTables: List<Table>) {
        val combined = _unfilteredTables.value + newTables
        val sortedTables =
            when (_tableListSortOrder.value) {
                TableListSortOrder.A_Z -> combined.sortedBy { it.name }
                TableListSortOrder.Z_A -> combined.sortedByDescending { it.name }
            }

        _unfilteredTables.update { sortedTables }
        _filteredTables.update { sortedTables }
        search(search.value)

        if (newTables.size == 1) {
            triggerScrollToTable(newTables[0])
        }
    }

    private fun removeTable(table: Table) {
        _unfilteredTables.update { tables -> tables.filter { it.uuid != table.uuid } }
        _filteredTables.update { tables -> tables.filter { it.uuid != table.uuid } }
    }

    private fun fetchTables() {
        tableJob?.cancel()
        tableJob =
            viewModelScope.launch {
                try {
                    _isLoading.update { true }
                    _loadingProgress.update { 0 }
                    _loadingStatus.update { "" }

                    val tables =
                        TableManager.loadTables { progress, status ->
                            _loadingProgress.update { progress }
                            _loadingStatus.update { status }
                        }

                    val sortedTables =
                        when (_tableListSortOrder.value) {
                            TableListSortOrder.A_Z -> tables.sortedBy { it.name }
                            TableListSortOrder.Z_A -> tables.sortedByDescending { it.name }
                        }.distinctBy { it.uuid }

                    withContext(Dispatchers.Main) {
                        _unfilteredTables.update { sortedTables }
                        _filteredTables.update { sortedTables }
                        search(search.value)
                    }
                } catch (e: Exception) {
                    withContext(Dispatchers.Main) {
                        _unfilteredTables.update { emptyList() }
                        _filteredTables.update { emptyList() }
                    }
                } finally {
                    _isLoading.update { false }
                }
            }
    }

    private fun loadSettings() {
        _tableListMode.value = TableListMode.fromInt(VPinballManager.loadValue(STANDALONE, "TableListMode", TableListMode.TWO_COLUMN.value))
        _tableListSortOrder.value = TableListSortOrder.fromInt(VPinballManager.loadValue(STANDALONE, "TableListSort", TableListSortOrder.A_Z.value))
    }
}
