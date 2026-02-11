package org.vpinball.app.ui.screens.landing

import android.graphics.ImageDecoder
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
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
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.focus.FocusRequester
import androidx.compose.ui.focus.focusRequester
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.TextRange
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.TextFieldValue
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import kotlin.math.floor
import kotlin.math.max
import kotlin.math.min
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import org.vpinball.app.Table
import org.vpinball.app.TableGridSize
import org.vpinball.app.TableViewMode
import org.vpinball.app.VPinballManager
import org.vpinball.app.jni.VPinballLogLevel
import org.vpinball.app.ui.screens.common.RoundedCard
import org.vpinball.app.ui.theme.VpxRed
import org.vpinball.app.util.resetImage
import org.vpinball.app.util.resetIni
import org.vpinball.app.util.resizeWithAspectFit
import org.vpinball.app.util.updateImage

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun TablesList(
    tables: List<Table>,
    viewMode: TableViewMode,
    gridSize: TableGridSize,
    onPlay: (table: Table) -> Unit,
    onRename: (table: Table, name: String) -> Unit,
    onViewScript: (table: Table) -> Unit,
    onShare: (table: Table) -> Unit,
    onDelete: (table: Table) -> Unit,
    modifier: Modifier = Modifier,
    lazyGridState: LazyGridState = rememberLazyGridState(),
    lazyListState: LazyListState = rememberLazyListState(),
    availableHeightOverride: Dp? = null,
) {
    val context = LocalContext.current
    val coroutineScope = rememberCoroutineScope()

    var currentTable by remember { mutableStateOf<Table?>(null) }

    var showRenameAlertDialog by remember { mutableStateOf(false) }
    var renameName by remember { mutableStateOf(TextFieldValue("")) }

    var showTableImageSheet by remember { mutableStateOf(false) }
    val tableImageSheetState: SheetState = rememberModalBottomSheetState(skipPartiallyExpanded = true)

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
                        val tableToUpdate = currentTable!!
                        coroutineScope.launch { withContext(Dispatchers.IO) { tableToUpdate.updateImage(resizedBitmap) } }
                    } catch (e: Exception) {
                        VPinballManager.log(VPinballLogLevel.ERROR, "Unable to change image: ${e.message}")
                    }
                }
            },
        )

    when (viewMode) {
        TableViewMode.LIST -> {
            LazyColumn(verticalArrangement = Arrangement.spacedBy(0.dp), modifier = modifier, state = lazyListState) {
                items(tables.size, key = { tables[it].uuid }) { index ->
                    val table = tables[index]
                    Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
                        if (index == 0) {
                            HorizontalDivider()
                        }

                        TableRowItem(
                            table = table,
                            onPlay = onPlay,
                            onRename = {
                                currentTable = table
                                renameName = renameName.copy(text = table.name)
                                showRenameAlertDialog = true
                            },
                            onTableImage = {
                                currentTable = table
                                showTableImageSheet = true
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
        else -> {
            BoxWithConstraints(modifier = modifier) {
                val maxWidth = this.maxWidth
                val maxHeight = availableHeightOverride ?: this.maxHeight

                val layout =
                    computeColumns(containerWidth = maxWidth - 32.dp, availableHeight = (maxHeight - 32.dp).coerceAtLeast(60.dp), gridSize = gridSize)

                LazyVerticalGrid(
                    columns = GridCells.Fixed(layout.columns),
                    verticalArrangement = Arrangement.spacedBy(layout.gap),
                    horizontalArrangement = Arrangement.spacedBy(layout.gap),
                    modifier = Modifier.fillMaxWidth().padding(horizontal = 16.dp, vertical = 16.dp),
                    state = lazyGridState,
                ) {
                    items(tables.size, key = { tables[it].uuid }) {
                        val table = tables[it]
                        Box(modifier = Modifier.fillMaxWidth(), contentAlignment = androidx.compose.ui.Alignment.Center) {
                            Box(modifier = Modifier.width(layout.cardWidth).height(layout.cardWidth * 1.5f)) {
                                TableGridItem(
                                    table = table,
                                    onPlay = onPlay,
                                    onRename = {
                                        currentTable = table
                                        renameName = renameName.copy(text = table.name)
                                        showRenameAlertDialog = true
                                    },
                                    onTableImage = {
                                        currentTable = table
                                        showTableImageSheet = true
                                    },
                                    onViewScript = { onViewScript(table) },
                                    onShare = { onShare(table) },
                                    onReset = { table.resetIni() },
                                    onDelete = { onDelete(table) },
                                )
                            }
                        }
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

    if (showTableImageSheet) {
        ModalBottomSheet(
            onDismissRequest = { showTableImageSheet = false },
            sheetState = tableImageSheetState,
            containerColor = MaterialTheme.colorScheme.surface,
        ) {
            Column(modifier = Modifier.padding(16.dp)) {
                RoundedCard {
                    TextButton(
                        onClick = {
                            showTableImageSheet = false
                            photoPickerLauncher.launch("image/*")
                        },
                        modifier = Modifier.fillMaxWidth(),
                    ) {
                        Text(text = "Photo Library", color = Color.VpxRed, style = MaterialTheme.typography.bodyLarge, fontWeight = FontWeight.Normal)
                    }

                    HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                    TextButton(
                        onClick = {
                            showTableImageSheet = false
                            val tableToReset = currentTable!!
                            coroutineScope.launch { withContext(Dispatchers.IO) { tableToReset.resetImage() } }
                        },
                        modifier = Modifier.fillMaxWidth(),
                    ) {
                        Text(text = "Reset", color = Color.VpxRed, style = MaterialTheme.typography.bodyLarge, fontWeight = FontWeight.Normal)
                    }
                }

                Spacer(modifier = Modifier.height(8.dp))

                RoundedCard {
                    TextButton(onClick = { showTableImageSheet = false }, modifier = Modifier.fillMaxWidth()) {
                        Text(text = "Cancel", color = Color.VpxRed, style = MaterialTheme.typography.titleMedium, fontWeight = FontWeight.SemiBold)
                    }
                }
            }
        }
    }
}

data class GridLayout(val columns: Int, val cardWidth: Dp, val gap: Dp)

private const val BASE_GAP = 12f
private const val RATIO = 2f / 3f
private const val MIN_READABLE_WIDTH = 120f
private const val MIN_FLOOR_REGULAR = 48f
private const val MIN_FLOOR_COMPACT = 40f

fun heightFactor(gridSize: TableGridSize): Float =
    when (gridSize) {
        TableGridSize.SMALL -> 0.72f
        TableGridSize.MEDIUM -> 0.88f
        TableGridSize.LARGE -> 1.0f
    }

data class TierResult(val small: Int, val medium: Int, val large: Int, val maxWidthFromHeight: Float)

fun computeTiers(containerWidth: Dp, availableHeight: Dp, gap: Dp, minFloor: Float): TierResult {
    val baseCap = max(60f, availableHeight.value) * RATIO

    fun calculateColumns(capFactor: Float): Int {
        var minWidth = MIN_READABLE_WIDTH
        val effectiveCap = baseCap * capFactor
        var effectiveMin = min(minWidth, effectiveCap)
        var columns = floor(((containerWidth.value + gap.value) / (effectiveMin + gap.value))).toInt()
        while (columns < 3 && minWidth > minFloor) {
            minWidth -= 6
            effectiveMin = min(minWidth, effectiveCap)
            columns = floor(((containerWidth.value + gap.value) / (effectiveMin + gap.value))).toInt()
        }
        return columns.coerceAtLeast(1)
    }

    val smallColumnsRaw = calculateColumns(0.72f)
    val mediumColumnsRaw = calculateColumns(0.88f)
    val largeColumnsRaw = calculateColumns(1.0f)

    var smallColumns = max(3, smallColumnsRaw)
    var mediumColumns = min(mediumColumnsRaw, smallColumns - 1)
    if (mediumColumns < 2) mediumColumns = max(2, smallColumns - 1)
    var largeColumns = min(largeColumnsRaw, mediumColumns - 1)
    if (largeColumns < 1) largeColumns = 1

    return TierResult(smallColumns, mediumColumns, largeColumns, baseCap)
}

fun cardWidthForColumns(columns: Int, containerWidth: Dp, heightCap: Dp, gridSize: TableGridSize, gap: Dp): Dp {
    val widthPerColumn = (containerWidth - gap * max(columns - 1, 0)) / max(columns, 1)
    var width = if (widthPerColumn < heightCap) widthPerColumn else heightCap
    if (columns == 1) {
        val factor =
            when (gridSize) {
                TableGridSize.SMALL -> 0.86f
                TableGridSize.MEDIUM -> 0.94f
                TableGridSize.LARGE -> 1.0f
            }
        val maxWidth = containerWidth * factor
        width = if (width < maxWidth) width else maxWidth
    }
    return width
}

fun computeColumns(containerWidth: Dp, availableHeight: Dp, gridSize: TableGridSize): GridLayout {
    val baseGap = BASE_GAP.dp
    val gap = if (availableHeight < 420.dp) 8.dp else baseGap
    val minFloor = if (availableHeight < 420.dp) MIN_FLOOR_COMPACT else MIN_FLOOR_REGULAR

    val tiers = computeTiers(containerWidth, availableHeight, gap, minFloor)

    val effectiveSmall = min(tiers.small, 6)
    var effectiveMedium = min(tiers.medium, effectiveSmall - 1)
    if (effectiveMedium < 2) effectiveMedium = max(1, effectiveSmall - 1)
    var effectiveLarge = min(tiers.large, effectiveMedium - 1)
    if (effectiveLarge < 1) effectiveLarge = 1

    val columns =
        when (gridSize) {
            TableGridSize.SMALL -> effectiveSmall
            TableGridSize.MEDIUM -> effectiveMedium
            TableGridSize.LARGE -> effectiveLarge
        }

    val heightCap = max(60f, availableHeight.value).dp * RATIO * heightFactor(gridSize)
    val cardWidth = cardWidthForColumns(columns, containerWidth, heightCap, gridSize, gap)

    return GridLayout(columns = columns, cardWidth = cardWidth, gap = gap)
}
