package org.vpinball.app.data.converter

import androidx.room.TypeConverter
import java.util.UUID

class UUIDConverter {
    @TypeConverter fun fromUUID(value: UUID?): String? = value?.toString()

    @TypeConverter fun fromString(value: String?): UUID? = UUID.fromString(value)
}
