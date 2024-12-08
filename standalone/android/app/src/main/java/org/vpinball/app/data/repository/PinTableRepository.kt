package org.vpinball.app.data.repository

import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.flowOf
import org.vpinball.app.data.dao.PinTableDao
import org.vpinball.app.data.entity.PinTable

interface PinTableRepository {
    fun getAllSorted(isAscending: Boolean): Flow<List<PinTable>>

    fun getById(id: String): Flow<PinTable>

    suspend fun insert(table: PinTable)

    suspend fun update(table: PinTable)

    suspend fun delete(table: PinTable)
}

class DefaultPinTableRepository(private val dao: PinTableDao) : PinTableRepository {
    override fun getAllSorted(isAscending: Boolean): Flow<List<PinTable>> {
        return if (isAscending) {
            dao.getAllSortedByNameAscThenModifiedAtDesc()
        } else {
            dao.getAllSortedByNameDescThenModifiedAtDesc()
        }
    }

    override fun getById(id: String): Flow<PinTable> = dao.getById(id)

    override suspend fun insert(table: PinTable) = dao.insert(table)

    override suspend fun update(table: PinTable) = dao.update(table)

    override suspend fun delete(table: PinTable) = dao.delete(table)
}

object TestPinTableRepository : PinTableRepository {
    override fun getAllSorted(isAscending: Boolean): Flow<List<PinTable>> = flowOf(emptyList())

    override fun getById(id: String): Flow<PinTable> = flowOf()

    override suspend fun insert(table: PinTable) = Unit

    override suspend fun update(table: PinTable) = Unit

    override suspend fun delete(table: PinTable) = Unit
}
