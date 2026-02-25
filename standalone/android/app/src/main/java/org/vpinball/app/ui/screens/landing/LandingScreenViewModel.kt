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
import org.vpinball.app.TableGridSize
import org.vpinball.app.TableListSortOrder
import org.vpinball.app.TableManager
import org.vpinball.app.TableViewMode
import org.vpinball.app.VPinballManager
import org.vpinball.app.VPinballModel
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

        private val _errorTrigger = MutableSharedFlow<String>(extraBufferCapacity = 1)
        val errorTrigger: SharedFlow<String> = _errorTrigger.asSharedFlow()

        fun triggerError(message: String) {
            _errorTrigger.tryEmit(message)
        }

        private val _openUriTrigger = MutableSharedFlow<android.net.Uri>(extraBufferCapacity = 1)
        val openUriTrigger: SharedFlow<android.net.Uri> = _openUriTrigger.asSharedFlow()

        fun triggerOpenUri(uri: android.net.Uri) {
            _openUriTrigger.tryEmit(uri)
        }
    }

    lateinit var vpinballModel: VPinballModel

    private var tableJob: Job? = null

    private val _filteredTables = MutableStateFlow(emptyList<Table>())
    val filteredTables: StateFlow<List<Table>> = _filteredTables

    private val _tableViewMode = MutableStateFlow(TableViewMode.GRID)
    val tableViewMode: StateFlow<TableViewMode> = _tableViewMode

    private val _tableGridSize = MutableStateFlow(TableGridSize.MEDIUM)
    val tableGridSize: StateFlow<TableGridSize> = _tableGridSize

    private val _tableListSortOrder = MutableStateFlow(TableListSortOrder.A_Z)
    val tableListSortOrder: StateFlow<TableListSortOrder> = _tableListSortOrder

    private val _search = MutableStateFlow("")
    val search: StateFlow<String> = _search

    private val _errorMessage = MutableStateFlow<String?>(null)
    val errorMessage: StateFlow<String?> = _errorMessage

    private val _importUri = MutableStateFlow<android.net.Uri?>(null)
    val importUri: StateFlow<android.net.Uri?> = _importUri

    private val _isFetchingTables = MutableStateFlow(false)
    val isFetchingTables: StateFlow<Boolean> = _isFetchingTables

    private val _fetchProgress = MutableStateFlow(0)
    val fetchProgress: StateFlow<Int> = _fetchProgress

    private val _fetchStatus = MutableStateFlow("")
    val fetchStatus: StateFlow<String> = _fetchStatus

    init {
        loadSettings()

        viewModelScope.launch { refreshTrigger.collect { refreshTables() } }

        viewModelScope.launch { updateTableTrigger.collect { table -> updateSingleTable(table) } }

        viewModelScope.launch { addTablesTrigger.collect { tables -> addNewTables(tables) } }

        viewModelScope.launch { removeTableTrigger.collect { table -> removeTable(table) } }

        viewModelScope.launch { errorTrigger.collect { message -> setError(message) } }

        viewModelScope.launch { openUriTrigger.collect { uri -> _importUri.update { uri } } }
    }

    fun initialize(vpinballModel: VPinballModel) {
        this.vpinballModel = vpinballModel
        if (vpinballModel.tables.isEmpty()) {
            fetchTables()
        }
    }

    fun setError(message: String) {
        _errorMessage.update { message }
    }

    fun clearError() {
        _errorMessage.update { null }
    }

    fun clearImportUri() {
        _importUri.update { null }
    }

    fun setTableViewMode(mode: TableViewMode) {
        _tableViewMode.update { mode }
        VPinballManager.saveValue(STANDALONE, "TableViewMode", mode.value)
    }

    fun setTableGridSize(size: TableGridSize) {
        _tableGridSize.update { size }
        VPinballManager.saveValue(STANDALONE, "TableGridSize", size.value)
    }

    fun setTableSortOrder(order: TableListSortOrder) {
        _tableListSortOrder.update { order }
        VPinballManager.saveValue(STANDALONE, "TableListSort", order.value)

        val sortedTables =
            when (order) {
                TableListSortOrder.A_Z -> vpinballModel.tables.sortedBy { it.name }
                TableListSortOrder.Z_A -> vpinballModel.tables.sortedByDescending { it.name }
            }

        vpinballModel.tables = sortedTables
        _filteredTables.update { sortedTables }
        search(search.value)
    }

    fun search(query: String) {
        _search.update { query }
        val tables = vpinballModel.tables
        if (query.isEmpty() || query.isBlank()) {
            _filteredTables.update { tables.toList() }
        } else {
            _filteredTables.update { tables.filter { it.name.lowercase().contains(query.trim().lowercase()) } }
        }
    }

    fun refreshTables() {
        fetchTables()
    }

    private fun updateSingleTable(updatedTable: Table) {
        vpinballModel.tables = vpinballModel.tables.map { if (it.uuid == updatedTable.uuid) updatedTable else it }
        _filteredTables.update { tables -> tables.map { if (it.uuid == updatedTable.uuid) updatedTable else it } }
    }

    private fun addNewTables(newTables: List<Table>) {
        val combined = vpinballModel.tables + newTables
        val sortedTables =
            when (_tableListSortOrder.value) {
                TableListSortOrder.A_Z -> combined.sortedBy { it.name }
                TableListSortOrder.Z_A -> combined.sortedByDescending { it.name }
            }

        vpinballModel.tables = sortedTables
        _filteredTables.update { sortedTables }
        search(search.value)

        if (newTables.size == 1) {
            triggerScrollToTable(newTables[0])
        }
    }

    private fun removeTable(table: Table) {
        vpinballModel.tables = vpinballModel.tables.filter { it.uuid != table.uuid }
        _filteredTables.update { tables -> tables.filter { it.uuid != table.uuid } }
    }

    private fun fetchTables() {
        tableJob?.cancel()
        tableJob =
            viewModelScope.launch {
                try {
                    _isFetchingTables.update { true }
                    _fetchProgress.update { 0 }
                    _fetchStatus.update { "" }

                    val tables =
                        TableManager.loadTables { progress, status ->
                            _fetchProgress.update { progress }
                            _fetchStatus.update { status }
                        }

                    val sortedTables =
                        when (_tableListSortOrder.value) {
                            TableListSortOrder.A_Z -> tables.sortedBy { it.name }
                            TableListSortOrder.Z_A -> tables.sortedByDescending { it.name }
                        }.distinctBy { it.uuid }

                    withContext(Dispatchers.Main) {
                        vpinballModel.tables = sortedTables
                        _filteredTables.update { sortedTables }
                        search(search.value)
                    }
                } catch (e: Exception) {
                    withContext(Dispatchers.Main) {
                        vpinballModel.tables = emptyList()
                        _filteredTables.update { emptyList() }
                    }
                } finally {
                    _isFetchingTables.update { false }
                }
            }
    }

    private fun loadSettings() {
        _tableViewMode.value = TableViewMode.fromInt(VPinballManager.loadValue(STANDALONE, "TableViewMode", TableViewMode.GRID.value))
        _tableGridSize.value = TableGridSize.fromInt(VPinballManager.loadValue(STANDALONE, "TableGridSize", TableGridSize.MEDIUM.value))
        _tableListSortOrder.value = TableListSortOrder.fromInt(VPinballManager.loadValue(STANDALONE, "TableListSort", TableListSortOrder.A_Z.value))
    }
}
