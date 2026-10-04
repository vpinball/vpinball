package org.vpinball.app.ui.screens.landing

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.foundation.text.input.TextFieldLineLimits
import androidx.compose.foundation.text.input.TextFieldState
import androidx.compose.foundation.text.input.clearText
import androidx.compose.foundation.text.selection.TextSelectionColors
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.TextField
import androidx.compose.material3.TextFieldDefaults
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.focus.FocusManager
import androidx.compose.ui.focus.onFocusChanged
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import org.vpinball.app.R
import org.vpinball.app.ui.theme.DarkBlack
import org.vpinball.app.ui.theme.DarkGrey
import org.vpinball.app.ui.theme.VpxRed

@Composable
fun LandingScreenSearchBar(
    searchTextFieldState: TextFieldState,
    searchText: String,
    searchBarHeight: Dp,
    searchBarOpacity: Float,
    isSearching: Boolean,
    focusManager: FocusManager,
    onFocusChanged: (Boolean) -> Unit,
    modifier: Modifier = Modifier,
) {
    Row(
        modifier =
            modifier
                .fillMaxWidth()
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
            modifier = Modifier.weight(1f).onFocusChanged { focusState -> onFocusChanged(focusState.isFocused) },
        )
        if (isSearching) {
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
}
