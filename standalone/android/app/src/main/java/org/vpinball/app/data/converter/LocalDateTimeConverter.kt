package org.vpinball.app.data.converter

import androidx.room.TypeConverter
import kotlinx.datetime.Instant
import kotlinx.datetime.LocalDateTime
import kotlinx.datetime.TimeZone
import kotlinx.datetime.toInstant
import kotlinx.datetime.toLocalDateTime

class LocalDateTimeConverter {
    @TypeConverter fun fromLocalDateTime(value: LocalDateTime?): Long? = value?.toInstant(TimeZone.UTC)?.epochSeconds

    @TypeConverter
    fun fromLong(value: Long?): LocalDateTime? = if (value != null) Instant.fromEpochMilliseconds(value).toLocalDateTime(TimeZone.UTC) else null
}
