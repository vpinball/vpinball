package org.vpinball.app.data

import androidx.room.Database
import androidx.room.RoomDatabase
import androidx.room.TypeConverters
import org.vpinball.app.data.converter.LocalDateTimeConverter
import org.vpinball.app.data.converter.UUIDConverter
import org.vpinball.app.data.dao.PinTableDao
import org.vpinball.app.data.entity.PinTable

@Database(entities = [PinTable::class], version = 1)
@TypeConverters(UUIDConverter::class, LocalDateTimeConverter::class)
abstract class AppDatabase : RoomDatabase() {
    abstract fun pinTableDao(): PinTableDao
}
