package org.vpinball.app.di

import androidx.room.Room
import org.koin.android.ext.koin.androidContext
import org.koin.core.module.dsl.viewModelOf
import org.koin.dsl.bind
import org.koin.dsl.module
import org.vpinball.app.Constants
import org.vpinball.app.VPinballViewModel
import org.vpinball.app.data.AppDatabase
import org.vpinball.app.data.repository.DefaultPinTableRepository
import org.vpinball.app.data.repository.PinTableRepository
import org.vpinball.app.ui.screens.landing.LandingScreenViewModel
import org.vpinball.app.ui.screens.settings.SettingsViewModel

val appModule = module {
    single { Room.databaseBuilder(androidContext(), AppDatabase::class.java, Constants.DATABASE_NAME).build() }
    single { get<AppDatabase>().pinTableDao() }
    single { DefaultPinTableRepository(get()) } bind PinTableRepository::class

    viewModelOf(::VPinballViewModel)
    viewModelOf(::SettingsViewModel)
    viewModelOf(::LandingScreenViewModel)
}
