package org.vpinball.app.ui.screens.landing

import android.graphics.ImageDecoder
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.LazyListState
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.LazyGridState
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.lazy.grid.rememberLazyGridState
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.text.selection.TextSelectionColors
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.OutlinedTextFieldDefaults
import androidx.compose.material3.SheetState
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.rememberModalBottomSheetState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.focus.FocusRequester
import androidx.compose.ui.focus.focusRequester
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.TextRange
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.TextFieldValue
import androidx.compose.ui.unit.dp
import org.vpinball.app.TableListMode
import org.vpinball.app.VPinballManager
import org.vpinball.app.data.entity.PinTable
import org.vpinball.app.jni.VPinballLogLevel
import org.vpinball.app.ui.screens.common.RoundedCard
import org.vpinball.app.ui.theme.VpxRed
import org.vpinball.app.util.deleteArtwork
import org.vpinball.app.util.resetIni
import org.vpinball.app.util.resizeWithAspectFit
import org.vpinball.app.util.saveArtwork

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun TablesList(
    tables: List<PinTable>,
    mode: TableListMode,
    onPlay: (table: PinTable) -> Unit,
    onRename: (table: PinTable, name: String) -> Unit,
    onChangeArtwork: (table: PinTable) -> Unit,
    onViewScript: (table: PinTable) -> Unit,
    onShare: (table: PinTable) -> Unit,
    onDelete: (table: PinTable) -> Unit,
    modifier: Modifier = Modifier,
    lazyGridState: LazyGridState = rememberLazyGridState(),
    lazyListState: LazyListState = rememberLazyListState(),
) {
    val context = LocalContext.current

    var currentTable by remember { mutableStateOf<PinTable?>(null) }

    var showRenameAlertDialog by remember { mutableStateOf(false) }
    var renameName by remember { mutableStateOf(TextFieldValue("")) }

    var showChangeArtworkSheet by remember { mutableStateOf(false) }
    val changeArtworkSheetState: SheetState = rememberModalBottomSheetState(skipPartiallyExpanded = true)

    val focusRequester = remember { FocusRequester() }

    val photoPickerLauncher =
        rememberLauncherForActivityResult(
            contract = ActivityResultContracts.GetContent(),
            onResult = { uri ->
                if (uri != null) {
                    try {
                        val source = ImageDecoder.createSource(context.contentResolver, uri)
                        val bitmap = ImageDecoder.decodeBitmap(source) { decoder, _, _ -> decoder.isMutableRequired = true }
                        val resizedBitmap =
                            bitmap.resizeWithAspectFit(
                                newWidth = VPinballManager.getDisplaySize().width,
                                newHeight = VPinballManager.getDisplaySize().height,
                            )
                        currentTable!!.saveArtwork(resizedBitmap)
                        onChangeArtwork(currentTable!!)
                    } catch (e: Exception) {
                        VPinballManager.log(VPinballLogLevel.ERROR, "Unable to change artwork: ${e.message}")
                    }
                }
            },
        )

    when (mode) {
        TableListMode.TWO_COLUMN -> {
            LazyVerticalGrid(
                columns = GridCells.Fixed(2),
                verticalArrangement = Arrangement.spacedBy(10.dp),
                horizontalArrangement = Arrangement.spacedBy(10.dp),
                modifier = modifier,
                state = lazyGridState,
            ) {
                items(tables.size, key = { tables[it].uuid }) {
                    val table = tables[it]
                    TableListGridItem(
                        table = table,
                        onPlay = onPlay,
                        onRename = {
                            currentTable = table
                            renameName = renameName.copy(text = table.name)
                            showRenameAlertDialog = true
                        },
                        onChangeArtwork = {
                            currentTable = table
                            showChangeArtworkSheet = true
                        },
                        onViewScript = { onViewScript(table) },
                        onShare = { onShare(table) },
                        onReset = { table.resetIni() },
                        onDelete = { onDelete(table) },
                    )
                }
            }
        }

        TableListMode.THREE_COLUMN -> {
            LazyVerticalGrid(
                columns = GridCells.Fixed(3),
                verticalArrangement = Arrangement.spacedBy(10.dp),
                horizontalArrangement = Arrangement.spacedBy(10.dp),
                modifier = modifier,
                state = lazyGridState,
            ) {
                items(tables.size, key = { tables[it].uuid }) {
                    val table = tables[it]
                    TableListGridItem(
                        table = table,
                        onPlay = onPlay,
                        onRename = {
                            currentTable = table
                            renameName = renameName.copy(text = table.name)
                            showRenameAlertDialog = true
                        },
                        onChangeArtwork = {
                            currentTable = table
                            showChangeArtworkSheet = true
                        },
                        onViewScript = { onViewScript(table) },
                        onShare = { onShare(table) },
                        onReset = { table.resetIni() },
                        onDelete = { onDelete(table) },
                    )
                }
            }
        }

        TableListMode.LIST -> {
            LazyColumn(verticalArrangement = Arrangement.spacedBy(0.dp), modifier = modifier, state = lazyListState) {
                items(tables.size, key = { tables[it].uuid }) { index ->
                    val table = tables[index]
                    Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
                        if (index == 0) {
                            HorizontalDivider()
                        }

                        TableListRowItem(
                            table = table,
                            onPlay = onPlay,
                            onRename = {
                                currentTable = table
                                renameName = renameName.copy(text = table.name)
                                showRenameAlertDialog = true
                            },
                            onChangeArtwork = {
                                currentTable = table
                                showChangeArtworkSheet = true
                            },
                            onViewScript = { onViewScript(table) },
                            onShare = { onShare(table) },
                            onReset = { table.resetIni() },
                            onDelete = { onDelete(table) },
                        )

                        HorizontalDivider()
                    }
                }
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
                            cursorColor = Color.VpxRed,
                            selectionColors = TextSelectionColors(handleColor = Color.Transparent, backgroundColor = Color.VpxRed.copy(alpha = 0.5f)),
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
                TextButton(
                    onClick = {
                        onRename(currentTable!!, renameName.text)
                        showRenameAlertDialog = false
                    },
                    enabled = renameName.text.isNotBlank(),
                ) {
                    Text(
                        text = "OK",
                        color = if (renameName.text.isNotBlank()) Color.VpxRed else Color.Gray,
                        fontSize = MaterialTheme.typography.titleMedium.fontSize,
                        fontWeight = FontWeight.SemiBold,
                    )
                }
            },
            dismissButton = {
                TextButton(onClick = { showRenameAlertDialog = false }) {
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

    if (showChangeArtworkSheet) {
        ModalBottomSheet(
            onDismissRequest = { showChangeArtworkSheet = false },
            sheetState = changeArtworkSheetState,
            containerColor = MaterialTheme.colorScheme.surface,
        ) {
            Column(modifier = Modifier.padding(16.dp)) {
                RoundedCard {
                    TextButton(
                        onClick = {
                            showChangeArtworkSheet = false
                            photoPickerLauncher.launch("image/*")
                        },
                        modifier = Modifier.fillMaxWidth(),
                    ) {
                        Text(text = "Photo Library", color = Color.VpxRed, style = MaterialTheme.typography.bodyLarge, fontWeight = FontWeight.Normal)
                    }

                    HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                    TextButton(
                        onClick = {
                            showChangeArtworkSheet = false
                            currentTable!!.deleteArtwork()
                            onChangeArtwork(currentTable!!)
                        },
                        modifier = Modifier.fillMaxWidth(),
                    ) {
                        Text(text = "Reset", color = Color.VpxRed, style = MaterialTheme.typography.bodyLarge, fontWeight = FontWeight.Normal)
                    }
                }

                Spacer(modifier = Modifier.height(8.dp))

                RoundedCard {
                    TextButton(onClick = { showChangeArtworkSheet = false }, modifier = Modifier.fillMaxWidth()) {
                        Text(text = "Cancel", color = Color.VpxRed, style = MaterialTheme.typography.titleMedium, fontWeight = FontWeight.SemiBold)
                    }
                }
            }
        }
    }
}
