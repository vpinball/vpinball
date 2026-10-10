package org.vpinball.app.ui.screens.landing

import android.graphics.ImageDecoder
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.requiredWidth
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.lazy.LazyRow
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.GridItemSpan
import androidx.compose.foundation.lazy.grid.LazyGridScope
import androidx.compose.foundation.lazy.grid.LazyGridState
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.lazy.grid.rememberLazyGridState
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.text.selection.TextSelectionColors
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.OutlinedTextFieldDefaults
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.focus.FocusRequester
import androidx.compose.ui.focus.focusRequester
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.TextRange
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.TextFieldValue
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import org.vpinball.app.Table
import org.vpinball.app.TableGridSize
import org.vpinball.app.VPinballManager
import org.vpinball.app.jni.VPinballLogLevel
import org.vpinball.app.ui.screens.common.AlertButton
import org.vpinball.app.util.resetImage
import org.vpinball.app.util.resetIni
import org.vpinball.app.util.resizeWithAspectFit
import org.vpinball.app.util.updateImage

private val GRID_GAP = 12.dp
private val GRID_PADDING = 16.dp
const val RECENTLY_PLAYED_LIMIT = 6

fun List<Table>.recentlyPlayed(): List<Table> = filter { it.lastPlayedAt != null }.sortedByDescending { it.lastPlayedAt }.take(RECENTLY_PLAYED_LIMIT)

fun List<Table>.favorites(): List<Table> = filter { it.isFavorite }

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun TablesList(
    tables: List<Table>,
    filteredTables: List<Table>,
    isSearching: Boolean,
    gridSize: TableGridSize,
    onPlay: (table: Table) -> Unit,
    onRename: (table: Table, name: String) -> Unit,
    onViewScript: (table: Table) -> Unit,
    onShare: (table: Table) -> Unit,
    onDelete: (table: Table) -> Unit,
    modifier: Modifier = Modifier,
    topContentPadding: Dp = 12.dp,
    bottomContentPadding: Dp = 12.dp,
    libraryGridState: LazyGridState = rememberLazyGridState(),
    resultsGridState: LazyGridState = rememberLazyGridState(),
) {
    val context = LocalContext.current
    val coroutineScope = rememberCoroutineScope()

    var currentTable by remember { mutableStateOf<Table?>(null) }

    var showRenameAlertDialog by remember { mutableStateOf(false) }
    var renameName by remember { mutableStateOf(TextFieldValue("")) }

    var confirmAction by remember { mutableStateOf<TableConfirmAction?>(null) }

    val focusRequester = remember { FocusRequester() }

    val photoPickerLauncher =
        rememberLauncherForActivityResult(
            contract = ActivityResultContracts.GetContent(),
            onResult = { uri ->
                val tableToUpdate = currentTable
                if (uri != null && tableToUpdate != null) {
                    coroutineScope.launch(Dispatchers.IO) {
                        try {
                            val source = ImageDecoder.createSource(context.contentResolver, uri)
                            val bitmap = ImageDecoder.decodeBitmap(source) { decoder, _, _ -> decoder.isMutableRequired = true }
                            val resizedBitmap =
                                bitmap.resizeWithAspectFit(
                                    newWidth = VPinballManager.getDisplaySize().width,
                                    newHeight = VPinballManager.getDisplaySize().height,
                                )
                            tableToUpdate.updateImage(resizedBitmap)
                        } catch (e: Exception) {
                            VPinballManager.log(VPinballLogLevel.ERROR, "Unable to change image: ${e.message}")
                        }
                    }
                }
            },
        )

    val recent = remember(tables) { tables.recentlyPlayed() }
    val favorites = remember(tables) { tables.favorites() }

    BoxWithConstraints(modifier = modifier) {
        val columns = gridSize.columns(wide = maxWidth >= 600.dp, compactHeight = maxHeight < 480.dp)
        val cardWidth = (maxWidth - GRID_PADDING * 2 - GRID_GAP * (columns - 1)) / columns

        fun LazyGridScope.cards(cardTables: List<Table>) {
            items(cardTables.size, key = { cardTables[it].uuid }) { index ->
                val table = cardTables[index]
                TableGridItem(
                    table = table,
                    onPlay = onPlay,
                    onRename = {
                        currentTable = table
                        renameName = renameName.copy(text = table.name)
                        showRenameAlertDialog = true
                    },
                    onSetImage = {
                        currentTable = table
                        photoPickerLauncher.launch("image/*")
                    },
                    onViewScript = { onViewScript(table) },
                    onShare = { onShare(table) },
                    onResetImage = { confirmAction = TableConfirmAction.ResetImage(table) },
                    onResetSettings = { confirmAction = TableConfirmAction.ResetSettings(table) },
                    onDelete = { confirmAction = TableConfirmAction.Delete(table) },
                )
            }
        }

        if (isSearching) {
            LazyVerticalGrid(
                columns = GridCells.Fixed(columns),
                state = resultsGridState,
                contentPadding = PaddingValues(start = GRID_PADDING, top = topContentPadding, end = GRID_PADDING, bottom = bottomContentPadding),
                verticalArrangement = Arrangement.spacedBy(16.dp),
                horizontalArrangement = Arrangement.spacedBy(GRID_GAP),
                modifier = Modifier.fillMaxWidth(),
            ) {
                item(key = "results-header", span = { GridItemSpan(maxLineSpan) }) { SectionHeader(title = "Results", count = filteredTables.size) }
                cards(filteredTables)
            }
        } else {
            LazyVerticalGrid(
                columns = GridCells.Fixed(columns),
                state = libraryGridState,
                contentPadding = PaddingValues(start = GRID_PADDING, top = topContentPadding, end = GRID_PADDING, bottom = bottomContentPadding),
                verticalArrangement = Arrangement.spacedBy(16.dp),
                horizontalArrangement = Arrangement.spacedBy(GRID_GAP),
                modifier = Modifier.fillMaxWidth(),
            ) {
                if (recent.isNotEmpty()) {
                    item(key = "recent", span = { GridItemSpan(maxLineSpan) }) {
                        MarqueeRow(
                            title = "Recently Played",
                            tables = recent,
                            cardWidth = cardWidth,
                            rowWidth = maxWidth,
                            inRecentlyPlayed = true,
                            onPlay = onPlay,
                            onRename = { table ->
                                currentTable = table
                                renameName = renameName.copy(text = table.name)
                                showRenameAlertDialog = true
                            },
                            onSetImage = { table ->
                                currentTable = table
                                photoPickerLauncher.launch("image/*")
                            },
                            onViewScript = onViewScript,
                            onShare = onShare,
                            onResetImage = { table -> confirmAction = TableConfirmAction.ResetImage(table) },
                            onResetSettings = { table -> confirmAction = TableConfirmAction.ResetSettings(table) },
                            onDelete = { table -> confirmAction = TableConfirmAction.Delete(table) },
                        )
                    }
                }

                if (favorites.isNotEmpty()) {
                    item(key = "favorites", span = { GridItemSpan(maxLineSpan) }) {
                        MarqueeRow(
                            title = "Favorites",
                            count = favorites.size,
                            tables = favorites,
                            cardWidth = cardWidth,
                            rowWidth = maxWidth,
                            onPlay = onPlay,
                            onRename = { table ->
                                currentTable = table
                                renameName = renameName.copy(text = table.name)
                                showRenameAlertDialog = true
                            },
                            onSetImage = { table ->
                                currentTable = table
                                photoPickerLauncher.launch("image/*")
                            },
                            onViewScript = onViewScript,
                            onShare = onShare,
                            onResetImage = { table -> confirmAction = TableConfirmAction.ResetImage(table) },
                            onResetSettings = { table -> confirmAction = TableConfirmAction.ResetSettings(table) },
                            onDelete = { table -> confirmAction = TableConfirmAction.Delete(table) },
                        )
                    }
                }

                item(key = "all-header", span = { GridItemSpan(maxLineSpan) }) { SectionHeader(title = "All Tables", count = tables.size) }
                cards(tables)
            }
        }
    }

    if (showRenameAlertDialog) {
        AlertDialog(
            title = { Text(text = "Rename Table", style = MaterialTheme.typography.titleMedium) },
            text = {
                OutlinedTextField(
                    value = renameName,
                    onValueChange = { renameName = it },
                    colors =
                        OutlinedTextFieldDefaults.colors(
                            cursorColor = MaterialTheme.colorScheme.primary,
                            selectionColors =
                                TextSelectionColors(
                                    handleColor = MaterialTheme.colorScheme.primary,
                                    backgroundColor = MaterialTheme.colorScheme.primary.copy(alpha = 0.4f),
                                ),
                            focusedBorderColor = MaterialTheme.colorScheme.onSurfaceVariant,
                        ),
                    singleLine = true,
                    modifier = Modifier.fillMaxWidth().focusRequester(focusRequester),
                )
                LaunchedEffect(Unit) {
                    renameName = renameName.copy(selection = TextRange(renameName.text.length))
                    focusRequester.requestFocus()
                }
            },
            onDismissRequest = {},
            confirmButton = {
                AlertButton(text = "OK", enabled = renameName.text.isNotBlank()) {
                    currentTable?.let { onRename(it, renameName.text) }
                    showRenameAlertDialog = false
                }
            },
            dismissButton = { AlertButton(text = "Cancel") { showRenameAlertDialog = false } },
        )
    }

    confirmAction?.let { action ->
        AlertDialog(
            title = { Text(text = action.title, style = MaterialTheme.typography.titleMedium) },
            text = { Text(action.message) },
            onDismissRequest = { confirmAction = null },
            confirmButton = {
                AlertButton(text = action.confirmLabel, destructive = true) {
                    confirmAction = null
                    when (action) {
                        is TableConfirmAction.ResetImage -> coroutineScope.launch { withContext(Dispatchers.IO) { action.table.resetImage() } }
                        is TableConfirmAction.ResetSettings -> action.table.resetIni()
                        is TableConfirmAction.Delete -> onDelete(action.table)
                    }
                }
            },
            dismissButton = { AlertButton(text = "Cancel") { confirmAction = null } },
        )
    }
}

sealed class TableConfirmAction(val table: Table) {
    class ResetImage(table: Table) : TableConfirmAction(table)

    class ResetSettings(table: Table) : TableConfirmAction(table)

    class Delete(table: Table) : TableConfirmAction(table)

    val title: String
        get() =
            when (this) {
                is ResetImage -> "Reset Image?"
                is ResetSettings -> "Reset Settings?"
                is Delete -> "Delete Table?"
            }

    val message: String
        get() =
            when (this) {
                is ResetImage -> "The image for \"${table.name}\" will be removed."
                is ResetSettings -> "The saved settings for \"${table.name}\" will be removed."
                is Delete -> "\"${table.name}\" and its files will be permanently deleted."
            }

    val confirmLabel: String
        get() = if (this is Delete) "Delete" else "Reset"
}

@Composable
fun SectionHeader(title: String, count: Int? = null, modifier: Modifier = Modifier) {
    Row(modifier = modifier.fillMaxWidth().padding(horizontal = 4.dp), verticalAlignment = Alignment.Bottom) {
        Text(
            text = title,
            style = MaterialTheme.typography.headlineSmall,
            fontWeight = FontWeight.Bold,
            color = MaterialTheme.colorScheme.onBackground,
            modifier = Modifier.weight(1f),
        )
        if (count != null) {
            Text(
                text = count.toString(),
                style = MaterialTheme.typography.titleSmall,
                fontWeight = FontWeight.SemiBold,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
            )
        }
    }
}

@Composable
private fun MarqueeRow(
    title: String,
    tables: List<Table>,
    cardWidth: Dp,
    rowWidth: Dp,
    onPlay: (table: Table) -> Unit,
    onRename: (table: Table) -> Unit,
    onSetImage: (table: Table) -> Unit,
    onViewScript: (table: Table) -> Unit,
    onShare: (table: Table) -> Unit,
    onResetImage: (table: Table) -> Unit,
    onResetSettings: (table: Table) -> Unit,
    onDelete: (table: Table) -> Unit,
    count: Int? = null,
    inRecentlyPlayed: Boolean = false,
) {
    Column(verticalArrangement = Arrangement.spacedBy(14.dp)) {
        SectionHeader(title = title, count = count)

        LazyRow(
            horizontalArrangement = Arrangement.spacedBy(GRID_GAP),
            contentPadding = PaddingValues(horizontal = GRID_PADDING),
            modifier = Modifier.requiredWidth(rowWidth),
        ) {
            items(tables, key = { it.uuid }) { table ->
                TableGridItem(
                    table = table,
                    inRecentlyPlayed = inRecentlyPlayed,
                    onPlay = onPlay,
                    onRename = onRename,
                    onSetImage = onSetImage,
                    onViewScript = onViewScript,
                    onShare = onShare,
                    onResetImage = onResetImage,
                    onResetSettings = onResetSettings,
                    onDelete = onDelete,
                    modifier = Modifier.width(cardWidth),
                )
            }
        }
    }
}
