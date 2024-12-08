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
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
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
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.windowInsetsPadding
import androidx.compose.foundation.lazy.LazyListState
import androidx.compose.foundation.lazy.grid.LazyGridState
import androidx.compose.foundation.lazy.grid.rememberLazyGridState
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.foundation.text.input.TextFieldLineLimits
import androidx.compose.foundation.text.input.clearText
import androidx.compose.foundation.text.input.rememberTextFieldState
import androidx.compose.foundation.text.selection.TextSelectionColors
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.TextField
import androidx.compose.material3.TextFieldDefaults
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
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.focus.onFocusChanged
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.platform.LocalFocusManager
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.core.content.FileProvider
import androidx.core.net.toUri
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import dev.chrisbanes.haze.HazeState
import dev.chrisbanes.haze.haze
import java.io.File
import org.koin.compose.koinInject
import org.vpinball.app.R
import org.vpinball.app.TableListMode
import org.vpinball.app.VPinballManager
import org.vpinball.app.data.entity.PinTable
import org.vpinball.app.ui.screens.common.ProgressOverlay
import org.vpinball.app.ui.screens.settings.SettingsModalBottomSheet
import org.vpinball.app.ui.theme.DarkBlack
import org.vpinball.app.ui.theme.DarkGrey
import org.vpinball.app.ui.theme.LightBlack
import org.vpinball.app.ui.theme.VPinballTheme
import org.vpinball.app.ui.theme.VpxDarkYellow
import org.vpinball.app.ui.theme.VpxRed
import org.vpinball.app.util.FileUtils
import org.vpinball.app.util.hasScript
import org.vpinball.app.util.scriptFile

@OptIn(ExperimentalMaterial3Api::class, ExperimentalFoundationApi::class)
@Composable
fun LandingScreen(
    webServerURL: String,
    progress: MutableState<Int>,
    status: MutableState<String>,
    onTableImported: (uuid: String, path: String) -> Unit,
    onRenameTable: (table: PinTable, name: String) -> Unit,
    onChangeTableArtwork: (table: PinTable) -> Unit,
    onDeleteTable: (table: PinTable) -> Unit,
    onViewFile: (file: File) -> Unit,
    modifier: Modifier = Modifier,
    viewModel: LandingScreenViewModel = koinInject(),
) {
    val context = LocalContext.current
    val focusManager = LocalFocusManager.current

    var showSettingsDialog by remember { mutableStateOf(false) }

    val tableListMode by viewModel.tableListMode.collectAsStateWithLifecycle()
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

    var scrollToTableUuid by remember { mutableStateOf<String?>(null) }

    val lazyGridState: LazyGridState = rememberLazyGridState()
    val lazyListState: LazyListState = rememberLazyListState()

    val totalOffset by remember {
        derivedStateOf {
            when (tableListMode) {
                TableListMode.LIST -> lazyListState.run { (firstVisibleItemIndex * 100) + firstVisibleItemScrollOffset }
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

    LaunchedEffect(scrollToTableUuid, unfilteredTables) {
        val uuid = scrollToTableUuid
        if (uuid != null) {
            val idx = unfilteredTables.indexOfFirst { it.uuid == uuid }
            if (idx >= 0) {
                if (tableListMode == TableListMode.LIST) {
                    lazyListState.animateScrollToItem(idx)
                } else {
                    lazyGridState.animateScrollToItem(idx)
                }
                scrollToTableUuid = null
            }
        }
    }

    BackHandler(enabled = isSearching) {
        searchTextFieldState.clearText()
        focusManager.clearFocus()
    }

    SettingsModalBottomSheet(
        webServerURL = webServerURL,
        show = showSettingsDialog,
        onDismissRequest = { showSettingsDialog = false },
        onViewFile = onViewFile,
    )

    Scaffold(
        modifier = Modifier.fillMaxSize().haze(state = hazeState),
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
                                    onExampleTable = {
                                        showImportTableMenu = false

                                        importUri = File(VPinballManager.getFilesDir(), "assets/exampleTable.vpx").toUri()
                                        importFilename = FileUtils.filenameFromUri(context, importUri!!)
                                        showConfirmDialog = true
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
        Column(modifier = Modifier.padding(padding).fillMaxWidth()) {
            Row(
                modifier =
                    Modifier.fillMaxWidth()
                        .height(searchBarHeight)
                        .background(color = Color.DarkBlack)
                        .alpha(alpha = searchBarOpacity)
                        .padding(horizontal = 16.dp, vertical = 8.dp),
                verticalAlignment = Alignment.CenterVertically,
            ) {
                TextField(
                    state = searchTextFieldState,
                    leadingIcon = {
                        Icon(
                            painter = painterResource(id = R.drawable.img_sf_magnifyingglass),
                            contentDescription = "Search",
                            tint = Color.LightGray,
                            modifier = Modifier.size(18.dp),
                        )
                    },
                    placeholder = { Text(text = "Search", color = Color.Gray) },
                    trailingIcon = {
                        if (searchText.isNotEmpty()) {
                            IconButton(onClick = { searchTextFieldState.clearText() }) {
                                Icon(
                                    painter = painterResource(id = R.drawable.img_sf_xmark_circle_fill),
                                    contentDescription = "Clear Search",
                                    tint = Color.LightGray,
                                    modifier = Modifier.size(18.dp),
                                )
                            }
                        }
                    },
                    contentPadding = PaddingValues(start = 0.dp, top = 7.dp, end = 0.dp, bottom = 5.dp),
                    lineLimits = TextFieldLineLimits.SingleLine,
                    shape = RoundedCornerShape(8.dp),
                    colors =
                        TextFieldDefaults.colors(
                            cursorColor = Color.VpxRed,
                            selectionColors = TextSelectionColors(handleColor = Color.Transparent, backgroundColor = Color.VpxRed.copy(alpha = 0.5f)),
                            focusedIndicatorColor = Color.Transparent,
                            unfocusedIndicatorColor = Color.Transparent,
                            disabledIndicatorColor = Color.Transparent,
                            focusedContainerColor = Color.DarkGrey,
                            unfocusedContainerColor = Color.DarkGrey,
                            disabledContainerColor = Color.DarkGrey,
                            focusedTextColor = Color.White,
                            unfocusedTextColor = Color.White,
                            disabledTextColor = Color.White,
                        ),
                    onKeyboardAction = { focusManager.clearFocus() },
                    keyboardOptions = KeyboardOptions.Default.copy(imeAction = ImeAction.Search),
                    modifier = modifier.weight(1f).onFocusChanged { focusState -> searchIsFocused = focusState.isFocused },
                )
                if (searchIsFocused || searchText.isNotEmpty()) {
                    Spacer(modifier = Modifier.width(8.dp))
                    TextButton(
                        onClick = {
                            searchTextFieldState.clearText()
                            focusManager.clearFocus()
                        }
                    ) {
                        Text(
                            text = "Cancel",
                            color = Color.VpxRed,
                            fontSize = MaterialTheme.typography.titleMedium.fontSize,
                            fontWeight = FontWeight.SemiBold,
                        )
                    }
                }
            }

            if (unfilteredTables.isEmpty()) {
                EmptyTablesList(modifier = Modifier.fillMaxWidth().windowInsetsPadding(WindowInsets.safeContent))
            } else if (filteredTables.isEmpty()) {
                NoResultsTableList(modifier = Modifier.fillMaxWidth().windowInsetsPadding(WindowInsets.safeContent).imePadding())
            } else {
                TablesList(
                    tables = filteredTables,
                    mode = tableListMode,
                    onPlay = { table ->
                        focusManager.clearFocus()

                        VPinballManager.play(table)
                    },
                    onRename = onRenameTable,
                    onChangeArtwork = onChangeTableArtwork,
                    onViewScript = { table ->
                        if (table.hasScript()) {
                            onViewFile(table.scriptFile)
                        } else {
                            title = table.name
                            progress.value = 0
                            status.value = ""
                            showProgress = true

                            VPinballManager.extractScript(
                                table,
                                onComplete = {
                                    showProgress = false
                                    onViewFile(table.scriptFile)
                                },
                                onError = { showProgress = false },
                            )
                        }
                    },
                    onShare = { table ->
                        title = table.name
                        progress.value = 0
                        status.value = ""
                        showProgress = true

                        VPinballManager.share(
                            table,
                            onComplete = {
                                showProgress = false

                                val fileUri = FileProvider.getUriForFile(context, "${context.packageName}.fileprovider", it)
                                val shareIntent =
                                    Intent(Intent.ACTION_SEND).apply {
                                        type = "application/octet-stream"
                                        putExtra(Intent.EXTRA_STREAM, fileUri)
                                        addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
                                    }

                                context.startActivity(Intent.createChooser(shareIntent, "Share File: $it"))
                            },
                            onError = { showProgress = false },
                        )
                    },
                    onDelete = onDeleteTable,
                    modifier = Modifier.fillMaxWidth().padding(all = 5.dp).imePadding(),
                    lazyGridState = lazyGridState,
                    lazyListState = lazyListState,
                )
            }
        }
    }

    if (showConfirmDialog) {
        AlertDialog(
            title = { Text(text = "Confirm Import Table", style = MaterialTheme.typography.titleMedium) },
            text = { Text("Import \"${importFilename}\"?") },
            onDismissRequest = {},
            confirmButton = {
                TextButton(
                    onClick = {
                        showConfirmDialog = false

                        importUri?.let {
                            VPinballManager.importUri(
                                context = context,
                                uri = it,
                                onUpdate = { inProgress, inStatus ->
                                    title = importFilename
                                    progress.value = inProgress
                                    status.value = inStatus
                                    showProgress = true
                                },
                                onComplete = { uuid, path ->
                                    showProgress = false
                                    onTableImported(uuid, path)
                                    scrollToTableUuid = uuid
                                },
                                onError = { showProgress = false },
                            )
                        } ?: error("$importFilename was not found!")
                    }
                ) {
                    Text(
                        text = "OK",
                        color = Color.VpxRed,
                        fontSize = MaterialTheme.typography.titleMedium.fontSize,
                        fontWeight = FontWeight.SemiBold,
                    )
                }
            },
            dismissButton = {
                TextButton(onClick = { showConfirmDialog = false }) {
                    Text(
                        text = "Cancel",
                        color = Color.VpxRed,
                        fontSize = MaterialTheme.typography.titleMedium.fontSize,
                        fontWeight = FontWeight.SemiBold,
                    )
                }
            },
        )
    }

    if (showProgress) {
        Box(modifier = Modifier.fillMaxSize().background(Color.Black.copy(alpha = 0.1f)).pointerInput(Unit) {})

        ProgressOverlay(title = title, progress = progress.value, status = status.value, hazeState = hazeState)
    }
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
            onChangeTableArtwork = {},
            onDeleteTable = {},
            onTableImported = { _, _ -> },
            onViewFile = { _ -> },
            modifier = Modifier,
        )
    }
}
