package org.vpinball.app.ui.screens.landing

import android.content.Intent
import android.content.res.Configuration
import android.net.Uri
import androidx.activity.compose.BackHandler
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.animation.AnimatedVisibility
import androidx.compose.animation.core.animateFloatAsState
import androidx.compose.animation.fadeIn
import androidx.compose.animation.fadeOut
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.WindowInsets
import androidx.compose.foundation.layout.asPaddingValues
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.imePadding
import androidx.compose.foundation.layout.navigationBars
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.statusBars
import androidx.compose.foundation.lazy.grid.LazyGridState
import androidx.compose.foundation.lazy.grid.rememberLazyGridState
import androidx.compose.foundation.text.input.clearText
import androidx.compose.foundation.text.input.rememberTextFieldState
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.derivedStateOf
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalFocusManager
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.core.content.FileProvider
import androidx.core.net.toUri
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.lifecycle.viewModelScope
import dev.chrisbanes.haze.hazeSource
import dev.chrisbanes.haze.rememberHazeState
import java.io.File
import java.io.FileOutputStream
import kotlinx.coroutines.launch
import org.koin.androidx.compose.koinViewModel
import org.vpinball.app.Link
import org.vpinball.app.R
import org.vpinball.app.SAFFileSystem
import org.vpinball.app.Table
import org.vpinball.app.TableManager
import org.vpinball.app.VPinballManager
import org.vpinball.app.VPinballModel
import org.vpinball.app.ui.screens.common.AlertButton
import org.vpinball.app.ui.screens.common.CircleIconButton
import org.vpinball.app.ui.screens.common.ProgressOverlay
import org.vpinball.app.ui.screens.common.frostedEdge
import org.vpinball.app.ui.screens.settings.SettingsBottomSheet
import org.vpinball.app.ui.theme.AmbientBackground
import org.vpinball.app.ui.theme.VPinballTheme
import org.vpinball.app.ui.util.koinActivityViewModel
import org.vpinball.app.util.FileUtils
import org.vpinball.app.util.hasScriptFile

@Composable
fun LandingScreen(
    onViewFile: (file: File) -> Unit,
    modifier: Modifier = Modifier,
    vpinballModel: VPinballModel = koinActivityViewModel(),
    viewModel: LandingScreenViewModel = koinViewModel(),
) {
    val context = LocalContext.current
    val focusManager = LocalFocusManager.current
    val coroutineScope = rememberCoroutineScope()
    LaunchedEffect(Unit) { viewModel.initialize(vpinballModel) }

    val errorMessage by viewModel.errorMessage.collectAsStateWithLifecycle()

    val progress = remember { mutableIntStateOf(0) }
    val status = remember { mutableStateOf("") }

    var showSettingsDialog by remember { mutableStateOf(false) }

    val tableGridSize by viewModel.tableGridSize.collectAsStateWithLifecycle()
    var showTableListModeMenu by remember { mutableStateOf(false) }

    var showConfirmDialog by remember { mutableStateOf(false) }
    var importUri by remember { mutableStateOf<Uri?>(null) }
    var importFilename by remember { mutableStateOf<String?>(null) }

    var searchActive by remember { mutableStateOf(false) }
    val searchText by viewModel.search.collectAsStateWithLifecycle()

    val isSearching by remember { derivedStateOf { searchActive || searchText.isNotEmpty() } }

    val searchTextFieldState = rememberTextFieldState(searchText)

    val filteredTables by viewModel.filteredTables.collectAsStateWithLifecycle()
    val tables = vpinballModel.tables

    val isFetchingTables by viewModel.isFetchingTables.collectAsStateWithLifecycle()
    val fetchProgress by viewModel.fetchProgress.collectAsStateWithLifecycle()
    val fetchStatus by viewModel.fetchStatus.collectAsStateWithLifecycle()

    var scrollToTable by remember { mutableStateOf<Table?>(null) }

    val libraryGridState: LazyGridState = rememberLazyGridState()
    val resultsGridState: LazyGridState = rememberLazyGridState()

    var showProgress by remember { mutableStateOf(false) }
    var title by remember { mutableStateOf<String?>(null) }

    val hazeState = rememberHazeState()

    val launcher =
        rememberLauncherForActivityResult(contract = ActivityResultContracts.OpenDocument()) { uri ->
            uri?.let {
                FileUtils.filenameFromUri(context, uri)?.let { filename ->
                    if (FileUtils.hasValidExtension(filename, arrayOf(".vpx", ".zip", ".vpxz"))) {
                        importFilename = filename
                        importUri = uri
                        showConfirmDialog = true
                    } else {
                        viewModel.setError("Unable to import table.")
                    }
                }
            }
        }

    val openUri by viewModel.importUri.collectAsStateWithLifecycle()
    LaunchedEffect(openUri) {
        val uri = openUri ?: return@LaunchedEffect
        viewModel.clearImportUri()
        FileUtils.filenameFromUri(context, uri)?.let { filename ->
            if (FileUtils.hasValidExtension(filename, arrayOf(".vpx", ".zip", ".vpxz"))) {
                importFilename = filename
                importUri = uri
                showConfirmDialog = true
            } else {
                viewModel.setError("Unable to import table.")
            }
        }
    }

    LaunchedEffect(searchTextFieldState.text) { viewModel.search(searchTextFieldState.text.toString()) }

    LaunchedEffect(Unit) { LandingScreenViewModel.scrollToTableTrigger.collect { table -> scrollToTable = table } }

    LaunchedEffect(scrollToTable, tables) {
        val table = scrollToTable ?: return@LaunchedEffect
        val idx = tables.indexOfFirst { it.uuid == table.uuid }
        if (idx >= 0) {
            val headerItems = (if (tables.recentlyPlayed().isNotEmpty()) 1 else 0) + (if (tables.favorites().isNotEmpty()) 1 else 0) + 1
            libraryGridState.animateScrollToItem(headerItems + idx)
            scrollToTable = null
        }
    }

    fun endSearch() {
        searchTextFieldState.clearText()
        focusManager.clearFocus()
        searchActive = false
    }

    BackHandler(enabled = isSearching) { endSearch() }

    fun importBuiltInTable(name: String) {
        try {
            val assetFile = File(context.cacheDir, name)
            context.assets.open("assets/$name").use { input -> FileOutputStream(assetFile).use { output -> input.copyTo(output) } }
            importUri = assetFile.toUri()
            importFilename = name
            showConfirmDialog = true
        } catch (e: Exception) {
            viewModel.setError("Failed to load $name: ${e.message}")
        }
    }

    val statusBarPadding = WindowInsets.statusBars.asPaddingValues().calculateTopPadding()
    val topContentPadding = statusBarPadding + (if (isSearching) 0.dp else TopBarHeight) + 12.dp
    val bottomContentPadding = WindowInsets.navigationBars.asPaddingValues().calculateBottomPadding() + BottomBarHeight + 36.dp

    Box(modifier = modifier.fillMaxSize()) {
        Box(modifier = Modifier.fillMaxSize().hazeSource(hazeState)) {
            AmbientBackground()

            Column(modifier = Modifier.fillMaxSize()) {
                if (tables.isEmpty()) {
                    EmptyTablesList(modifier = Modifier.fillMaxWidth().padding(top = topContentPadding, bottom = bottomContentPadding))
                } else if (isSearching && filteredTables.isEmpty()) {
                    NoResultsTableList(
                        modifier = Modifier.fillMaxWidth().padding(top = topContentPadding, bottom = bottomContentPadding).imePadding()
                    )
                } else {
                    TablesList(
                        tables = tables,
                        filteredTables = filteredTables,
                        isSearching = isSearching,
                        gridSize = tableGridSize,
                        onPlay = { table ->
                            focusManager.clearFocus()
                            VPinballManager.play(tables.firstOrNull { it.uuid == table.uuid } ?: table)
                        },
                        onRename = { table, name -> viewModel.viewModelScope.launch { TableManager.getInstance().renameTable(table, name) } },
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

                                coroutineScope.launch {
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

                            coroutineScope.launch {
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

                            coroutineScope.launch {
                                TableManager.getInstance()
                                    .deleteTable(
                                        table = table,
                                        onProgress = { inProgress, inStatus ->
                                            progress.value = inProgress
                                            status.value = inStatus
                                        },
                                    )
                                showProgress = false
                            }
                        },
                        modifier = Modifier.fillMaxWidth().weight(1f).imePadding(),
                        topContentPadding = topContentPadding,
                        bottomContentPadding = bottomContentPadding,
                        libraryGridState = libraryGridState,
                        resultsGridState = resultsGridState,
                    )
                }
            }
        }

        val scrolled by remember { derivedStateOf { libraryGridState.canScrollBackward } }
        val frostAlpha by animateFloatAsState(if (scrolled) 1f else 0f, label = "top_bar_frost")

        AnimatedVisibility(visible = !isSearching, enter = fadeIn(), exit = fadeOut(), modifier = Modifier.align(Alignment.TopCenter)) {
            Box(modifier = Modifier.fillMaxWidth()) {
                val frostHeight = statusBarPadding + TopBarHeight + 44.dp
                Box(
                    modifier =
                        Modifier.fillMaxWidth()
                            .height(frostHeight)
                            .frostedEdge(hazeState, frostAlpha, solidFraction = (statusBarPadding + TopBarHeight + 4.dp) / frostHeight)
                )

                LandingTopBar(hazeState = hazeState, onSettings = { showSettingsDialog = true }) {
                    CircleIconButton(hazeState = hazeState, iconRes = R.drawable.img_sf_ellipsis, contentDescription = "View Options") {
                        showTableListModeMenu = true
                    }

                    TableListModeDropdownMenu(
                        expanded = showTableListModeMenu,
                        onDismissRequest = { showTableListModeMenu = false },
                        viewModel = viewModel,
                    )
                }
            }
        }

        LandingBottomBar(
            hazeState = hazeState,
            showSearch = tables.isNotEmpty(),
            searchActive = searchActive,
            onSearchActiveChange = { active -> if (active) searchActive = true else endSearch() },
            searchTextFieldState = searchTextFieldState,
            searchText = searchText,
            focusManager = focusManager,
            onFiles = { launcher.launch(arrayOf("*/*")) },
            onBlankTable = { importBuiltInTable("blankTable.vpx") },
            onExampleTable = { importBuiltInTable("exampleTable.vpx") },
            modifier = Modifier.align(Alignment.BottomCenter),
        )

        if (showConfirmDialog) {
            ImportConfirmDialog(
                filename = importFilename,
                onConfirm = {
                    showConfirmDialog = false
                    importUri?.let { uri ->
                        coroutineScope.launch {
                            TableManager.importTable(
                                uri = uri,
                                onUpdate = { inProgress, inStatus ->
                                    title = importFilename
                                    progress.value = inProgress
                                    status.value = inStatus
                                    showProgress = true
                                },
                                onComplete = { _, _ -> showProgress = false },
                                onError = { showProgress = false },
                            )
                        }
                    } ?: run { error("$importFilename was not found!") }
                },
                onDismiss = { showConfirmDialog = false },
            )
        }

        if (showProgress || isFetchingTables) {
            Box(modifier = Modifier.fillMaxSize().background(Color.Black.copy(alpha = 0.1f)).pointerInput(Unit) {})

            val displayTitle = if (isFetchingTables) "Loading Tables" else title
            val displayProgress = if (isFetchingTables) fetchProgress else progress.value
            val displayStatus = if (isFetchingTables) fetchStatus else status.value

            ProgressOverlay(title = displayTitle, progress = displayProgress, status = displayStatus, hazeState = hazeState)
        }

        errorMessage?.let { message ->
            AlertDialog(
                title = { Text(text = "TILT!", style = MaterialTheme.typography.titleMedium) },
                text = { Text(message) },
                onDismissRequest = {},
                confirmButton = { AlertButton(text = "OK") { viewModel.clearError() } },
                dismissButton = {
                    AlertButton(text = "Learn More") {
                        viewModel.clearError()
                        Link.TROUBLESHOOTING.open(context = context)
                    }
                },
            )
        }

        SettingsBottomSheet(
            webServerURL = vpinballModel.webServerURL ?: "",
            show = showSettingsDialog,
            onDismissRequest = { showSettingsDialog = false },
            onViewFile = onViewFile,
        )
    }
}

@Preview
@Preview(uiMode = Configuration.UI_MODE_NIGHT_YES)
@Composable
private fun PreviewLandingScreen() {
    VPinballTheme { LandingScreen(onViewFile = { _ -> }, modifier = Modifier) }
}
