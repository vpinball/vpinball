package org.vpinball.app.ui.screens.landing

import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.imePadding
import androidx.compose.foundation.layout.navigationBarsPadding
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.statusBarsPadding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.foundation.text.input.TextFieldLineLimits
import androidx.compose.foundation.text.input.TextFieldState
import androidx.compose.foundation.text.input.clearText
import androidx.compose.foundation.text.selection.TextSelectionColors
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.material3.TextField
import androidx.compose.material3.TextFieldDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.focus.FocusManager
import androidx.compose.ui.focus.FocusRequester
import androidx.compose.ui.focus.focusRequester
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.unit.dp
import dev.chrisbanes.haze.HazeState
import org.vpinball.app.R
import org.vpinball.app.ui.screens.common.CircleButtonSize
import org.vpinball.app.ui.screens.common.CircleIconButton
import org.vpinball.app.ui.screens.common.glassPill

val BottomBarHeight = CircleButtonSize
val TopBarHeight = 56.dp
val BottomBarPadding = 16.dp

@Composable
fun LandingTopBar(hazeState: HazeState, onSettings: () -> Unit, modifier: Modifier = Modifier, viewOptions: @Composable () -> Unit) {
    Box(modifier = modifier.fillMaxWidth().statusBarsPadding().padding(horizontal = BottomBarPadding).height(TopBarHeight)) {
        Image(
            painter = painterResource(id = R.drawable.img_vpinball_logo),
            contentDescription = null,
            modifier = Modifier.align(Alignment.Center).height(36.dp),
        )

        Box(modifier = Modifier.align(Alignment.CenterStart)) {
            CircleIconButton(hazeState = hazeState, iconRes = R.drawable.img_sf_gearshape, contentDescription = "Settings", onClick = onSettings)
        }

        Box(modifier = Modifier.align(Alignment.CenterEnd)) { viewOptions() }
    }
}

@Composable
fun LandingBottomBar(
    hazeState: HazeState,
    showSearch: Boolean,
    searchActive: Boolean,
    onSearchActiveChange: (Boolean) -> Unit,
    searchTextFieldState: TextFieldState,
    searchText: String,
    focusManager: FocusManager,
    onFiles: () -> Unit,
    onBlankTable: () -> Unit,
    onExampleTable: () -> Unit,
    modifier: Modifier = Modifier,
) {
    var showImportMenu by remember { mutableStateOf(false) }
    val focusRequester = remember { FocusRequester() }
    val expanded = showSearch && searchActive

    LaunchedEffect(expanded) {
        if (expanded) {
            focusRequester.requestFocus()
        }
    }

    val container = Color.Transparent
    val content = MaterialTheme.colorScheme.onSurface
    val hint = MaterialTheme.colorScheme.onSurfaceVariant

    Row(
        modifier = modifier.fillMaxWidth().navigationBarsPadding().imePadding().padding(horizontal = BottomBarPadding, vertical = 12.dp),
        verticalAlignment = Alignment.CenterVertically,
    ) {
        if (expanded) {
            TextField(
                state = searchTextFieldState,
                leadingIcon = {
                    Icon(
                        painter = painterResource(id = R.drawable.img_sf_magnifyingglass),
                        contentDescription = "Search",
                        tint = hint,
                        modifier = Modifier.size(18.dp),
                    )
                },
                placeholder = { Text(text = "Search Tables", color = hint) },
                trailingIcon = {
                    if (searchText.isNotEmpty()) {
                        IconButton(onClick = { searchTextFieldState.clearText() }) {
                            Icon(
                                painter = painterResource(id = R.drawable.img_sf_xmark_circle_fill),
                                contentDescription = "Clear Search",
                                tint = hint,
                                modifier = Modifier.size(18.dp),
                            )
                        }
                    }
                },
                contentPadding = PaddingValues(start = 0.dp, top = 7.dp, end = 0.dp, bottom = 5.dp),
                lineLimits = TextFieldLineLimits.SingleLine,
                shape = CircleShape,
                colors =
                    TextFieldDefaults.colors(
                        cursorColor = MaterialTheme.colorScheme.primary,
                        selectionColors =
                            TextSelectionColors(
                                handleColor = MaterialTheme.colorScheme.primary,
                                backgroundColor = MaterialTheme.colorScheme.primary.copy(alpha = 0.4f),
                            ),
                        focusedIndicatorColor = Color.Transparent,
                        unfocusedIndicatorColor = Color.Transparent,
                        disabledIndicatorColor = Color.Transparent,
                        focusedContainerColor = container,
                        unfocusedContainerColor = container,
                        disabledContainerColor = container,
                        focusedTextColor = content,
                        unfocusedTextColor = content,
                        disabledTextColor = content,
                    ),
                onKeyboardAction = { focusManager.clearFocus() },
                keyboardOptions = KeyboardOptions.Default.copy(imeAction = ImeAction.Search),
                modifier = Modifier.weight(1f).height(BottomBarHeight).glassPill(hazeState).focusRequester(focusRequester),
            )

            Spacer(modifier = Modifier.width(12.dp))

            CircleIconButton(hazeState = hazeState, iconRes = R.drawable.img_sf_xmark, contentDescription = "Cancel Search") {
                searchTextFieldState.clearText()
                focusManager.clearFocus()
                onSearchActiveChange(false)
            }
        } else {
            if (showSearch) {
                CircleIconButton(hazeState = hazeState, iconRes = R.drawable.img_sf_magnifyingglass, contentDescription = "Search") {
                    onSearchActiveChange(true)
                }
            }

            Spacer(modifier = Modifier.weight(1f))

            Box {
                CircleIconButton(hazeState = hazeState, iconRes = R.drawable.img_sf_plus, contentDescription = "Import Table") {
                    showImportMenu = true
                }

                ImportTableDropdownMenu(
                    expanded = showImportMenu,
                    onDismissRequest = { showImportMenu = false },
                    onFiles = {
                        showImportMenu = false
                        onFiles()
                    },
                    onBlankTable = {
                        showImportMenu = false
                        onBlankTable()
                    },
                    onExampleTable = {
                        showImportMenu = false
                        onExampleTable()
                    },
                )
            }
        }
    }
}
