package org.vpinball.app.ui.screens.settings

import androidx.compose.foundation.layout.WindowInsets
import androidx.compose.foundation.layout.WindowInsetsSides
import androidx.compose.foundation.layout.navigationBars
import androidx.compose.foundation.layout.only
import androidx.compose.foundation.layout.statusBars
import androidx.compose.foundation.layout.windowInsetsPadding
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.SheetState
import androidx.compose.material3.rememberModalBottomSheetState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Modifier
import androidx.compose.ui.input.nestedscroll.nestedScroll
import androidx.compose.ui.platform.rememberNestedScrollInteropConnection
import java.io.File
import kotlinx.coroutines.launch

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SettingsModalBottomSheet(
    webServerURL: String,
    show: Boolean,
    onDismissRequest: () -> Unit,
    onViewFile: (file: File) -> Unit,
    modifier: Modifier = Modifier,
    sheetState: SheetState = rememberModalBottomSheetState(skipPartiallyExpanded = true),
) {
    if (show) {
        val coroutineScope = rememberCoroutineScope()

        ModalBottomSheet(
            onDismissRequest = onDismissRequest,
            modifier =
                modifier
                    .windowInsetsPadding(WindowInsets.statusBars)
                    .windowInsetsPadding(WindowInsets.navigationBars.only(WindowInsetsSides.Top))
                    .nestedScroll(rememberNestedScrollInteropConnection()),
            sheetState = sheetState,
            containerColor = MaterialTheme.colorScheme.background,
        ) {
            val onDone: () -> Unit = {
                coroutineScope.launch {
                    sheetState.hide()
                    onDismissRequest()
                }
            }

            SettingsScreen(webServerURL = webServerURL, onDone = onDone, onViewFile = onViewFile)
        }
    }
}
