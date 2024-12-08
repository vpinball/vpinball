package org.vpinball.app.data.dao

import androidx.room.Dao
import androidx.room.Delete
import androidx.room.Insert
import androidx.room.Query
import androidx.room.Update
import kotlinx.coroutines.flow.Flow
import org.vpinball.app.data.entity.PinTable

@Dao
interface PinTableDao {
    @Query(
        """
        SELECT * FROM pintable 
        ORDER BY name COLLATE NOCASE ASC, modifiedAt DESC
    """
    )
    fun getAllSortedByNameAscThenModifiedAtDesc(): Flow<List<PinTable>>

    @Query(
        """
        SELECT * FROM pintable 
        ORDER BY name COLLATE NOCASE DESC, modifiedAt DESC
    """
    )
    fun getAllSortedByNameDescThenModifiedAtDesc(): Flow<List<PinTable>>

    @Query("SELECT * FROM pintable WHERE uuid == :id") fun getById(id: String): Flow<PinTable>

    @Insert suspend fun insert(table: PinTable)

    @Update suspend fun update(table: PinTable)

    @Delete suspend fun delete(table: PinTable)
}
