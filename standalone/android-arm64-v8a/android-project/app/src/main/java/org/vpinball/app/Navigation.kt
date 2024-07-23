package org.vpinball.app

import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.navigation.NavHostController
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable

enum class Screen {
    TABLES,
    DIR_SELECTION,
    SETTINGS
}

sealed class NavigationItem(val route: String) {
    data object Tables : NavigationItem(Screen.TABLES.name)
    data object Settings : NavigationItem(Screen.SETTINGS.name)
    data object DirSelection : NavigationItem(Screen.DIR_SELECTION.name)
}

@Composable
fun Navigator(
    modifier: Modifier = Modifier,
    controller: NavHostController,
    model: VPXViewModel
) {
    NavHost(
        modifier = modifier,
        navController = controller,
        startDestination = if (model.isConfigured.value) NavigationItem.Tables.route else NavigationItem.DirSelection.route
    ) {
        composable(NavigationItem.Tables.route) { Tables(controller, model) }
        composable(NavigationItem.Settings.route) { Settings(controller) }
        composable(NavigationItem.DirSelection.route) { VPXDirSelection(controller) }
    }
}