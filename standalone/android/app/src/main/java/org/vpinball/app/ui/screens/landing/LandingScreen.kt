package org.vpinball.app.ui.screens.landing

import android.content.Intent
import android.content.res.Configuration
import android.net.Uri
import androidx.activity.compose.BackHandler
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.ExperimentalFoundationApi
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.WindowInsets
import androidx.compose.foundation.layout.asPaddingValues
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.imePadding
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.safeContent
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.statusBars
import androidx.compose.foundation.layout.windowInsetsPadding
import androidx.compose.foundation.lazy.LazyListState
import androidx.compose.foundation.lazy.grid.LazyGridState
import androidx.compose.foundation.lazy.grid.rememberLazyGridState
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.text.input.clearText
import androidx.compose.foundation.text.input.rememberTextFieldState
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.Scaffold
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.MutableState
import androidx.compose.runtime.derivedStateOf
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.platform.LocalFocusManager
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.core.content.FileProvider
import androidx.core.net.toUri
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import dev.chrisbanes.haze.HazeState
import dev.chrisbanes.haze.haze
import java.io.File
import java.io.FileOutputStream
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import org.koin.androidx.compose.koinViewModel
import org.vpinball.app.R
import org.vpinball.app.SAFFileSystem
import org.vpinball.app.Table
import org.vpinball.app.TableManager
import org.vpinball.app.TableViewMode
import org.vpinball.app.VPinballManager
import org.vpinball.app.ui.screens.common.ProgressOverlay
import org.vpinball.app.ui.screens.settings.SettingsBottomSheet
import org.vpinball.app.ui.theme.DarkBlack
import org.vpinball.app.ui.theme.LightBlack
import org.vpinball.app.ui.theme.VPinballTheme
import org.vpinball.app.ui.theme.VpxDarkYellow
import org.vpinball.app.util.FileUtils
import org.vpinball.app.util.hasScriptFile

@OptIn(ExperimentalMaterial3Api::class, ExperimentalFoundationApi::class)
@Composable
fun LandingScreen(
    webServerURL: String,
    progress: MutableState<Int>,
    status: MutableState<String>,
    onTableImported: (uuid: String, path: String) -> Unit,
    onRenameTable: (table: Table, name: String) -> Unit,
    onTableImage: (table: Table) -> Unit,
    onDeleteTable: (table: Table) -> Unit,
    onViewFile: (file: File) -> Unit,
    onPlayTable: (table: Table) -> Unit,
    modifier: Modifier = Modifier,
    viewModel: LandingScreenViewModel = koinViewModel(),
) {
    val context = LocalContext.current
    val focusManager = LocalFocusManager.current

    var showSettingsDialog by remember { mutableStateOf(false) }

    val tableViewMode by viewModel.tableViewMode.collectAsStateWithLifecycle()
    val tableGridSize by viewModel.tableGridSize.collectAsStateWithLifecycle()
    var showTableListModeMenu by remember { mutableStateOf(false) }

    var showImportTableMenu by remember { mutableStateOf(false) }
    var showConfirmDialog by remember { mutableStateOf(false) }
    var importUri by remember { mutableStateOf<Uri?>(null) }
    var importFilename by remember { mutableStateOf<String?>(null) }

    var searchIsFocused by remember { mutableStateOf(false) }
    val searchText by viewModel.search.collectAsStateWithLifecycle()

    val isSearching by remember { derivedStateOf { searchIsFocused || searchText.isNotEmpty() } }

    val searchTextFieldState = rememberTextFieldState(searchText)

    val filteredTables by viewModel.filteredTables.collectAsStateWithLifecycle()
    val unfilteredTables by viewModel.unfilteredTables.collectAsStateWithLifecycle()

    val isViewModelLoading by viewModel.isLoading.collectAsStateWithLifecycle()
    val viewModelLoadingProgress by viewModel.loadingProgress.collectAsStateWithLifecycle()
    val viewModelLoadingStatus by viewModel.loadingStatus.collectAsStateWithLifecycle()

    var scrollToTable by remember { mutableStateOf<Table?>(null) }

    val lazyGridState: LazyGridState = rememberLazyGridState()
    val lazyListState: LazyListState = rememberLazyListState()

    val totalOffset by remember {
        derivedStateOf {
            when (tableViewMode) {
                TableViewMode.LIST -> lazyListState.run { (firstVisibleItemIndex * 100) + firstVisibleItemScrollOffset }
                else -> lazyGridState.run { (firstVisibleItemIndex * 100) + firstVisibleItemScrollOffset }
            }.toFloat()
        }
    }

    val maxSearchBarHeight = 58.dp
    val density = LocalDensity.current

    val searchBarHeight by remember {
        derivedStateOf {
            if (isSearching) {
                maxSearchBarHeight
            } else {
                val offsetDp = with(density) { totalOffset.toDp() }
                (maxSearchBarHeight - offsetDp).coerceIn(0.dp, maxSearchBarHeight)
            }
        }
    }

    val searchBarOpacity by remember {
        derivedStateOf {
            if (isSearching) {
                1f
            } else {
                val offsetDp = with(density) { totalOffset.toDp() }
                (1f - offsetDp / (maxSearchBarHeight / 2)).coerceIn(0f, 1f)
            }
        }
    }

    var showProgress by remember { mutableStateOf(false) }
    var title by remember { mutableStateOf<String?>(null) }

    val hazeState = remember { HazeState() }

    val launcher =
        rememberLauncherForActivityResult(contract = ActivityResultContracts.OpenDocument()) { uri ->
            uri?.let {
                FileUtils.filenameFromUri(context, uri)?.let { filename ->
                    if (FileUtils.hasValidExtension(filename, arrayOf(".vpx", ".zip", ".vpxz"))) {
                        importFilename = filename
                        importUri = uri
                        showConfirmDialog = true
                    } else {
                        VPinballManager.showError("Unable to import table.")
                    }
                }
            }
        }

    LaunchedEffect(searchTextFieldState.text) { viewModel.search(searchTextFieldState.text.toString()) }

    LaunchedEffect(Unit) { LandingScreenViewModel.scrollToTableTrigger.collect { table -> scrollToTable = table } }

    LaunchedEffect(scrollToTable, unfilteredTables) {
        val table = scrollToTable
        if (table != null) {
            val idx = unfilteredTables.indexOfFirst { it.uuid == table.uuid }
            if (idx >= 0) {
                if (tableViewMode == TableViewMode.LIST) {
                    lazyListState.animateScrollToItem(idx)
                } else {
                    lazyGridState.animateScrollToItem(idx)
                }
                scrollToTable = null
            }
        }
    }

    BackHandler(enabled = isSearching) {
        searchTextFieldState.clearText()
        focusManager.clearFocus()
    }

    Scaffold(
        modifier = modifier.fillMaxSize().haze(state = hazeState),
        containerColor = Color.LightBlack,
        topBar = {
            if (!isSearching) {
                Box(modifier = Modifier.fillMaxWidth()) {
                    TopAppBar(
                        title = {},
                        navigationIcon = {
                            IconButton(onClick = { showSettingsDialog = true }) {
                                Icon(
                                    painter = painterResource(id = R.drawable.img_sf_gearshape),
                                    contentDescription = null,
                                    tint = Color.VpxDarkYellow,
                                    modifier = Modifier.size(22.dp),
                                )
                            }
                        },
                        actions = {
                            Box {
                                IconButton(onClick = { showTableListModeMenu = true }) {
                                    Icon(
                                        painter = painterResource(id = R.drawable.img_sf_ellipsis_circle),
                                        contentDescription = null,
                                        tint = Color.VpxDarkYellow,
                                        modifier = Modifier.size(22.dp),
                                    )
                                }

                                TableListModeDropdownMenu(
                                    expanded = showTableListModeMenu,
                                    onDismissRequest = { showTableListModeMenu = false },
                                    viewModel = viewModel,
                                )
                            }

                            Box {
                                IconButton(onClick = { showImportTableMenu = true }) {
                                    Icon(
                                        painter = painterResource(id = R.drawable.img_sf_plus),
                                        contentDescription = null,
                                        tint = Color.VpxDarkYellow,
                                        modifier = Modifier.size(22.dp),
                                    )
                                }

                                ImportTableDropdownMenu(
                                    expanded = showImportTableMenu,
                                    onDismissRequest = { showImportTableMenu = false },
                                    onFiles = {
                                        showImportTableMenu = false

                                        launcher.launch(arrayOf("*/*"))
                                    },
                                    onBlankTable = {
                                        showImportTableMenu = false
                                        try {
                                            val assetFile = File(context.cacheDir, "blankTable.vpx")
                                            context.assets.open("assets/blankTable.vpx").use { input ->
                                                FileOutputStream(assetFile).use { output -> input.copyTo(output) }
                                            }
                                            importUri = assetFile.toUri()
                                            importFilename = "blankTable.vpx"
                                            showConfirmDialog = true
                                        } catch (e: Exception) {
                                            VPinballManager.showError("Failed to load blank table: ${e.message}")
                                        }
                                    },
                                    onExampleTable = {
                                        showImportTableMenu = false

                                        try {
                                            val assetFile = File(context.cacheDir, "exampleTable.vpx")
                                            context.assets.open("assets/exampleTable.vpx").use { input ->
                                                FileOutputStream(assetFile).use { output -> input.copyTo(output) }
                                            }
                                            importUri = assetFile.toUri()
                                            importFilename = "exampleTable.vpx"
                                            showConfirmDialog = true
                                        } catch (e: Exception) {
                                            VPinballManager.showError("Failed to load example table: ${e.message}")
                                        }
                                    },
                                )
                            }
                        },
                        colors = TopAppBarDefaults.topAppBarColors(containerColor = Color.DarkBlack),
                    )

                    Box(
                        modifier = Modifier.fillMaxWidth().padding(WindowInsets.statusBars.asPaddingValues()).height(56.dp),
                        contentAlignment = Alignment.Center,
                    ) {
                        Image(
                            painter = painterResource(id = R.drawable.img_vpinball_logo),
                            contentDescription = null,
                            modifier = Modifier.height(44.dp).padding(top = 8.dp),
                        )
                    }
                }
            } else {
                TopAppBar(
                    title = {},
                    navigationIcon = {},
                    expandedHeight = 0.dp,
                    colors = TopAppBarDefaults.topAppBarColors(containerColor = Color.DarkBlack),
                )
            }
        },
    ) { padding ->
        BoxWithConstraints(modifier = Modifier.padding(padding).fillMaxWidth()) {
            val searchBarAllowance = 24.dp
            val stableAvailableHeight = (this.maxHeight - maxSearchBarHeight - searchBarAllowance).coerceAtLeast(60.dp)
            Column(modifier = Modifier.fillMaxWidth()) {
                LandingScreenSearchBar(
                    searchTextFieldState = searchTextFieldState,
                    searchText = searchText,
                    searchBarHeight = searchBarHeight,
                    searchBarOpacity = searchBarOpacity,
                    isSearching = isSearching,
                    focusManager = focusManager,
                    onFocusChanged = { searchIsFocused = it },
                )

                if (unfilteredTables.isEmpty()) {
                    EmptyTablesList(modifier = Modifier.fillMaxWidth().windowInsetsPadding(WindowInsets.safeContent))
                } else if (filteredTables.isEmpty()) {
                    NoResultsTableList(modifier = Modifier.fillMaxWidth().windowInsetsPadding(WindowInsets.safeContent).imePadding())
                } else {
                    TablesList(
                        tables = filteredTables,
                        viewMode = tableViewMode,
                        gridSize = tableGridSize,
                        onPlay = { table ->
                            focusManager.clearFocus()
                            onPlayTable(table)
                        },
                        onRename = onRenameTable,
                        onViewScript = { table ->
                            val viewScriptFile: () -> Unit = {
                                val file =
                                    if (SAFFileSystem.isUsingSAF()) {
                                        val tempFile = File(context.cacheDir, "view_script_${table.uuid}.vbs")
                                        val inputStream = SAFFileSystem.openInputStream(table.scriptPath)
                                        if (inputStream != null) {
                                            FileOutputStream(tempFile).use { output -> inputStream.use { input -> input.copyTo(output) } }
                                            tempFile
                                        } else {
                                            null
                                        }
                                    } else {
                                        table.scriptURL
                                    }
                                file?.let { onViewFile(it) }
                            }

                            if (table.hasScriptFile()) {
                                viewScriptFile()
                            } else {
                                title = table.name
                                progress.value = 0
                                status.value = "Extracting script"
                                showProgress = true

                                CoroutineScope(Dispatchers.Main).launch {
                                    TableManager.extractTableScript(
                                        table,
                                        onProgress = { inProgress, inStatus ->
                                            progress.value = inProgress
                                            status.value = inStatus
                                        },
                                        onComplete = {
                                            showProgress = false
                                            viewScriptFile()
                                        },
                                        onError = { showProgress = false },
                                    )
                                }
                            }
                        },
                        onShare = { table ->
                            title = table.name
                            progress.value = 0
                            status.value = "Exporting table"
                            showProgress = true

                            CoroutineScope(Dispatchers.Main).launch {
                                TableManager.shareTable(
                                    table,
                                    onProgress = { inProgress, inStatus ->
                                        progress.value = inProgress
                                        status.value = inStatus
                                    },
                                    onComplete = { path ->
                                        showProgress = false

                                        val file = File(path)
                                        val fileUri = FileProvider.getUriForFile(context, "${context.packageName}.fileprovider", file)
                                        val shareIntent =
                                            Intent(Intent.ACTION_SEND).apply {
                                                type = "application/octet-stream"
                                                putExtra(Intent.EXTRA_STREAM, fileUri)
                                                addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
                                            }

                                        context.startActivity(Intent.createChooser(shareIntent, "Share File: ${file.name}"))
                                    },
                                    onError = { showProgress = false },
                                )
                            }
                        },
                        onDelete = { table ->
                            title = table.name
                            progress.value = 0
                            status.value = "Deleting table"
                            showProgress = true

                            CoroutineScope(Dispatchers.Main).launch {
                                TableManager.getInstance()
                                    .deleteTable(
                                        table = table,
                                        onProgress = { inProgress, inStatus ->
                                            progress.value = inProgress
                                            status.value = inStatus
                                        },
                                    )
                                showProgress = false
                                onDeleteTable(table)
                            }
                        },
                        modifier = Modifier.fillMaxWidth().weight(1f).padding(all = 5.dp).imePadding(),
                        lazyGridState = lazyGridState,
                        lazyListState = lazyListState,
                        availableHeightOverride = stableAvailableHeight,
                    )
                }
            }
        }
    }

    if (showConfirmDialog) {
        ImportConfirmDialog(
            filename = importFilename,
            onConfirm = {
                showConfirmDialog = false
                importUri?.let { uri ->
                    CoroutineScope(Dispatchers.Main).launch {
                        TableManager.importTable(
                            uri = uri,
                            onUpdate = { inProgress, inStatus ->
                                title = importFilename
                                progress.value = inProgress
                                status.value = inStatus
                                showProgress = true
                            },
                            onComplete = { uuid, path ->
                                showProgress = false
                                onTableImported(uuid, path)
                            },
                            onError = { showProgress = false },
                        )
                    }
                } ?: run { error("$importFilename was not found!") }
            },
            onDismiss = { showConfirmDialog = false },
        )
    }

    if (showProgress || isViewModelLoading) {
        Box(modifier = Modifier.fillMaxSize().background(Color.Black.copy(alpha = 0.1f)).pointerInput(Unit) {})

        val displayTitle = if (isViewModelLoading) "Loading Tables" else title
        val displayProgress = if (isViewModelLoading) viewModelLoadingProgress else progress.value
        val displayStatus = if (isViewModelLoading) viewModelLoadingStatus else status.value

        ProgressOverlay(title = displayTitle, progress = displayProgress, status = displayStatus, hazeState = hazeState)
    }

    SettingsBottomSheet(
        webServerURL = webServerURL,
        show = showSettingsDialog,
        onDismissRequest = { showSettingsDialog = false },
        onViewFile = onViewFile,
    )
}

@Preview
@Preview(uiMode = Configuration.UI_MODE_NIGHT_YES)
@Composable
private fun PreviewLandingScreen() {
    val progress = remember { mutableIntStateOf(0) }
    val status = remember { mutableStateOf("") }
    VPinballTheme {
        LandingScreen(
            webServerURL = "test.url",
            progress = progress,
            status = status,
            onRenameTable = { _, _ -> },
            onTableImage = {},
            onDeleteTable = {},
            onTableImported = { _, _ -> },
            onViewFile = { _ -> },
            onPlayTable = { _ -> },
            modifier = Modifier,
        )
    }
}
