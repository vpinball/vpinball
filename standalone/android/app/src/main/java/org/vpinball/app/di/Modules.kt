package org.vpinball.app.di

import org.koin.core.module.dsl.viewModelOf
import org.koin.dsl.module
import org.vpinball.app.VPinballViewModel
import org.vpinball.app.ui.screens.landing.LandingScreenViewModel
import org.vpinball.app.ui.screens.settings.SettingsViewModel

val appModule = module {
    viewModelOf(::VPinballViewModel)
    viewModelOf(::SettingsViewModel)
    viewModelOf(::LandingScreenViewModel)
}
